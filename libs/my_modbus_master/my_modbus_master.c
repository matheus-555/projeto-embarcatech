#include "my_modbus_master.h"
#include "my_std_debug.h"
#include "global.h"
#include <string.h>
#include <stdio.h>
#include <stdatomic.h>
#include "pico/stdlib.h"
#include "lwip/tcp.h"


#define BYTES_MAX_COIL 256UL
#define BYTES_MAX_HOLDING 128UL



//==========================================================================
// VARIAVEIS PRIVADAS
//==========================================================================
static struct tcp_pcb *modbus_pcb;
static uint8_t coils[BYTES_MAX_COIL];                 // Até 256 bits
static uint16_t holding_registers[BYTES_MAX_HOLDING]; // Até 128 registradores
static volatile atomic_bool data_ready = 0;           // Flag para sincronização

//==========================================================================
// PROTOTIPOS DAS FUNCOES PRIVADAS
//==========================================================================

static err_t modbus_tcp_recv_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static err_t modbus_tcp_connected_callback(void *arg, struct tcp_pcb *tpcb, err_t err);
static void modbus_send_request_write(uint8_t function, uint16_t addr, uint16_t value);
static void modbus_send_request_read(uint8_t function, uint16_t start, uint16_t count);
static void modbus_wait_response();

//==========================================================================
// FUNCOES PUBLICAS
//==========================================================================

// OK
// Lê Coils e retorna diretamente
uint8_t modbus_read_coil(my_modbus_master_t *obj, uint16_t coil_addr)
{
    modbus_send_request_read(MY_MODBUS_FUNCTION_READ_COIL, coil_addr, 1);
    modbus_wait_response();
    return (coils[0] & 1);
}

// OK
// Lê Holding Registers e retorna diretamente
uint16_t modbus_read_holding_register(my_modbus_master_t *obj, uint16_t reg_addr)
{
    modbus_send_request_read(MY_MODBUS_FUNCTION_READ_HOLDING_REGISTER, reg_addr, 1);
    modbus_wait_response();
    return holding_registers[0];
}

// OK
// Lê Input Status e retorna diretamente
uint8_t modbus_read_input_status(my_modbus_master_t *obj, uint16_t input_addr)
{
    modbus_send_request_read(MY_MODBUS_FUNCTION_READ_INPUT_STATUS, input_addr, 1);

    modbus_wait_response();
    
    return (coils[0] & 1);
}

// OK
// Lê Input Register e retorna o valor lido
uint16_t modbus_read_input_register(my_modbus_master_t *obj, uint16_t reg_addr)
{
    modbus_send_request_read(MY_MODBUS_FUNCTION_READ_INPUT_REGISTER, reg_addr, 1);

    modbus_wait_response();

    return holding_registers[0]; // Mesmo buffer da leitura 0x03
}

// OK
// Escreve na Coil (0x05)
void modbus_write_coil(my_modbus_master_t *obj, uint16_t coil_addr, uint8_t value)
{
    modbus_send_request_write(MY_MODBUS_FUNCTION_WRITE_COIL, coil_addr, value);

    modbus_wait_response();
}

// OK
// Escreve na Holding Register (0x06)
void modbus_write_holding_register(my_modbus_master_t *obj, uint16_t register_addr, uint16_t value)
{
    modbus_send_request_write(MY_MODBUS_FUNCTION_WRITE_HOLDING_REGISTER, register_addr, value);

    // Aguarda a resposta
    modbus_wait_response();
}

// OK
//
my_std_ret_t modbus_connect(my_modbus_master_t *obj)
{
    modbus_pcb = tcp_new();

    if(modbus_pcb == NULL)
    {
        #if GLOBAL_DEBUG_ENABLED
        MY_STD_DEBUG_MSG("Error in tcp_new, pointer is null");
        #endif

        return MY_STD_RET_FAIL;
    }

    ip_addr_t ip;
    ipaddr_aton(obj->server_ip, &ip);

    #if GLOBAL_DEBUG_ENABLED
    printf("Connecting in server Modbus TCP %s:%d...\n", obj->server_ip, obj->server_port);
    #endif

    tcp_connect(modbus_pcb, &ip, obj->server_port, modbus_tcp_connected_callback);

    return MY_STD_RET_OK;
}

//==========================================================================
// FUNCOES PRIVADAS
//==========================================================================

// OK
// --- Callback para receber a resposta Modbus
static err_t modbus_tcp_recv_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    if (p == NULL)
    {
        tcp_close(tpcb);

        return ERR_ABRT;
    }

    err_t ret = ERR_OK;
    uint8_t *data = (uint8_t *) p->payload;
    uint8_t function_code;

    if(p->len < 7)
    {
        ret = ERR_ABRT;
        goto end_function;
    }

    function_code = data[7];

    switch (function_code)
    {
        case MY_MODBUS_FUNCTION_READ_COIL:
        case MY_MODBUS_FUNCTION_READ_INPUT_STATUS:
            memcpy(coils, &data[9], data[8]); // Copia os bits lidos
            goto case_end;
        case MY_MODBUS_FUNCTION_READ_HOLDING_REGISTER:
        case MY_MODBUS_FUNCTION_READ_INPUT_REGISTER:
            for (register int i = 0; i < data[8] / 2; i++)
                holding_registers[i] = (data[9 + 2 * i] << 8) | data[10 + 2 * i];

            goto case_end;

        case MY_MODBUS_FUNCTION_WRITE_COIL:
        case MY_MODBUS_FUNCTION_WRITE_HOLDING_REGISTER:
            goto case_end;

        case_end:
            atomic_store(&data_ready, 1);
            break;
    }

    end_function:

    pbuf_free(p);

    return ret;
}

// OK
// Conecta ao servidor Modbus TCP
static err_t modbus_tcp_connected_callback(void *arg, struct tcp_pcb *tpcb, err_t err)
{
    if (err != ERR_OK)
    {
        return ERR_ABRT;
    }

    tcp_recv(tpcb, modbus_tcp_recv_callback);

    return ERR_OK;
}

// OK
// Envia requisição de leitura Modbus
static void modbus_send_request_read(uint8_t function, uint16_t start, uint16_t count)
{
    uint8_t request[12];
    static uint16_t transaction_id = 0;

    // Prepara o cabecalho TCP/IP
    transaction_id++;
    request[0] = transaction_id >> 8;   // Transação HIGH Byte
    request[1] = transaction_id & 0xFF; // Transação LOW Byte
    request[2] = 0;                     // Protocolo
    request[3] = 0;                     // Protocolo
    request[4] = 0;                     // Comprimento
    request[5] = 6;                     // Comprimento -> Tamanho do pedido
    request[6] = 1;                     // Endereço do servidor modbus (SLAVE ID)
    request[7] = function;              // Função modbus
    request[8] = start >> 8;            // Endereço inicial HIGH BYTE
    request[9] = start & 0xFF;          // Endereço inicial LOW BYTE
    request[10] = count >> 8;           // Numero de registro a serem lidos HIGH
    request[11] = count & 0xFF;         // Numero de registro a serem lidos LOW

    // Despacha os dados
    tcp_write(modbus_pcb, request, sizeof(request), TCP_WRITE_FLAG_COPY);

    // Força o despacho imediato OBS: pode crashar a execucao
    // tcp_output(modbus_pcb);
}

// OK
// 📤 Envia requisição de escrita Modbus
static void modbus_send_request_write(uint8_t function, uint16_t addr, uint16_t value)
{

    uint8_t request[12];
    uint16_t transaction_id = 0x0001; // ID da transação (ajuste conforme necessário)
    uint16_t protocol_id = 0x0000;    // ID do protocolo (Modbus)
    uint16_t length = 6;              // Tamanho da mensagem

    // Prepara o cabecalho TCP/IP
    request[0] = transaction_id >> 8;          // ID da transação HIGH
    request[1] = transaction_id & 0xFF;        // ID da transação LOW
    request[2] = protocol_id >> 8;             // Protocolo ID HIGH
    request[3] = protocol_id & 0xFF;           // Protocolo ID LOW
    request[4] = length >> 8;                  // Comprimento HGH
    request[5] = length & 0xFF;                // Comprimento LOW

    // Corpo da requisição Modbus
    request[6] = 1;        // Unit Identifier
    request[7] = function; // Função (escrever coil ou holding register)
    request[8] = (addr >> 8) & 0xFF;
    request[9] = addr & 0xFF; // Endereço do registro (coil ou holding register)

    if (function == 0x05)
    {                                             // Escrever em uma coil (Function Code 0x05)
        request[10] = (value == 0) ? 0x00 : 0xFF; // Valor 0 ou 1 para coils
        request[11] = 0x00;
    }
    else if (function == 0x06)
    {                                      // Escrever em um holding register (Function Code 0x06)
        request[10] = (value >> 8) & 0xFF; // Parte alta do valor
        request[11] = value & 0xFF;        // Parte baixa do valor
    }

    // Envia a requisição Modbus
    tcp_write(modbus_pcb, request, sizeof(request), TCP_WRITE_FLAG_COPY);

    // Força o envio dos dados
    // tcp_output(modbus_pcb);
}

// OK
// Espera a resposta Modbus (bloqueia até receber)
static void modbus_wait_response()
{
    atomic_store(&data_ready, 0);

    while (!data_ready);
}
