#ifndef __MY_MODBUS_H__
#define __MY_MODBUS_H__


#include "my_std_ret.h"
#include <stdint.h>
#include <stdbool.h>

// // Monta a requisição para escrever em um Holding Register
// uint8_t request[12];    // Buffer para armazenar a requisição
// uint16_t transaction_id = 1;  // ID da transação (pode ser incrementado em cada chamada)

// // Prepara o pedido Modbus (Função 0x06)
// request[0] = transaction_id >> 8;        // ID da transação (2 bytes)
// request[1] = transaction_id & 0xFF;
// request[2] = 0;                          // Protocólo (2 bytes)
// request[3] = 0;
// request[4] = 0;                          // Comprimento (2 bytes)
// request[5] = 6;
// request[6] = 1;                          // Endereço do dispositivo (unit_id)
// request[7] = 0x06;                       // Função 0x06 (escrita no holding register)
// request[8] = register_addr >> 8;         // Endereço do Holding Register (2 bytes)
// request[9] = register_addr & 0xFF;
// request[10] = value >> 8;                // Valor do Holding Register (parte alta)
// request[11] = value & 0xFF;              // Valor do Holding Register (parte baixa)

// // Envia a requisição Modbus (requisição de escrita em Holding Register)
// tcp_write(modbus_pcb, request, sizeof(request), TCP_WRITE_FLAG_COPY);

// // Força o envio dos dados
// tcp_output(modbus_pcb);

//==========================================================================
// ENUMS
//==========================================================================
typedef enum
{
    MY_MODBUS_FUNCTION_READ_COIL              = 1UL,
    MY_MODBUS_FUNCTION_READ_INPUT_STATUS      = 2UL,
    MY_MODBUS_FUNCTION_READ_HOLDING_REGISTER  = 3UL,
    MY_MODBUS_FUNCTION_READ_INPUT_REGISTER    = 4UL,
    MY_MODBUS_FUNCTION_WRITE_COIL             = 5UL,
    MY_MODBUS_FUNCTION_WRITE_HOLDING_REGISTER = 6UL,
} my_modbus_master_function_enum;

//==========================================================================
// STRUCT
//==========================================================================

struct _my_modbus_control_t;

typedef struct
{
    const char *server_ip;
    const uint16_t server_port;
    const uint16_t slave_id;
} my_modbus_master_t;

//==========================================================================
// PROTOTIPOS DAS FUNCOES PUBLICAS
//==========================================================================
// my_std_ret_t modbus_connect(my_modbus_master_t *obj);
// uint8_t modbus_read_coil(uint16_t coil_addr);
// uint16_t modbus_read_holding_register(uint16_t reg_addr);
// uint8_t modbus_read_input_status(uint16_t input_addr);
// uint16_t modbus_read_input_register(uint16_t reg_addr);
// void modbus_write_coil(uint16_t coil_addr, uint8_t value);
// void modbus_write_holding_register(uint16_t register_addr, uint16_t value);



my_std_ret_t modbus_connect(my_modbus_master_t *obj);
uint8_t modbus_read_coil(my_modbus_master_t *obj, uint16_t coil_addr);
uint16_t modbus_read_holding_register(my_modbus_master_t *obj, uint16_t reg_addr);
uint8_t modbus_read_input_status(my_modbus_master_t *obj, uint16_t input_addr);
uint16_t modbus_read_input_register(my_modbus_master_t *obj, uint16_t reg_addr);
void modbus_write_coil(my_modbus_master_t *obj, uint16_t coil_addr, uint8_t value);
void modbus_write_holding_register(my_modbus_master_t *obj, uint16_t register_addr, uint16_t value);

#endif