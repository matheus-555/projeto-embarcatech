#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/sync.h"

#include "my_std_debug.h"
#include "my_configs.h"
#include "my_wifi.h"
#include "my_gpio.h"
#include "my_dma.h"
#include "my_modbus_master.h"

#define HYSTERY_ADC_VALUE 50UL

int main()
{
  stdio_init_all();

  sleep_ms(500);

  my_gpio_t btn = {
      .dir = GPIO_IN,
      .pin = MY_GPIO_IN_BTN_A,
      .res_pull = GPIO_RES_PULL_UP};

  my_modbus_master_t modbus = {
      .server_ip = MY_CONFIGS_IP_MODBUS,
      .server_port = MY_CONFIGS_PORT_MODBUS,
      .slave_id = MY_CONFIGS_SID_MODBUS};

  int adc_channel_init[] = {MY_GPIO_ADC_IN_CH_JOYSTICK_Y, MY_GPIO_ADC_IN_CH_JOYSTICK_X};
  int adc_channel_0[1] = {0};
  int adc_channel_1[1] = {0};


    
  bool last_state_gpio;
  int last_state_adc_ch0, last_state_adc_ch1;

  while (my_wifi_init(MY_CONFIGS_WIFI_SSID, MY_CONFIGS_WIFI_PASSWORD) != MY_STD_RET_OK)
  {
    sleep_ms(100);
  }

  my_gpio_init(&btn);
  my_dma_init(adc_channel_init, sizeof(adc_channel_init)/sizeof(adc_channel_init[0]));

  MY_STD_DEBUG_ERROR_CHECK(modbus_connect(&modbus));

  last_state_gpio = gpio_get(btn.pin);
  my_dma_get_value(MY_GPIO_ADC_IN_CH_JOYSTICK_X, 1, &last_state_adc_ch0);
  my_dma_get_value(MY_GPIO_ADC_IN_CH_JOYSTICK_Y, 1, &last_state_adc_ch1);

  while (1)
  {
    if (last_state_gpio != gpio_get(btn.pin) )
    {
      last_state_gpio = gpio_get(btn.pin);
      modbus_write_coil(&modbus, 0, last_state_gpio);
    }

    my_dma_get_value(MY_GPIO_ADC_IN_CH_JOYSTICK_X, sizeof(adc_channel_0)/sizeof(adc_channel_0[0]), adc_channel_0);

    if ( last_state_adc_ch0 < adc_channel_0[0]-HYSTERY_ADC_VALUE || last_state_adc_ch0 > adc_channel_0[0]+HYSTERY_ADC_VALUE )
    {
      last_state_adc_ch0 = adc_channel_0[0];
      modbus_write_holding_register(&modbus, 3, last_state_adc_ch0);
    }

    my_dma_get_value(MY_GPIO_ADC_IN_CH_JOYSTICK_Y, sizeof(adc_channel_1)/sizeof(adc_channel_1[0]), adc_channel_1);

    if ( last_state_adc_ch1 < adc_channel_1[0]-HYSTERY_ADC_VALUE || last_state_adc_ch1 > adc_channel_1[0]+HYSTERY_ADC_VALUE )
    {
      last_state_adc_ch1 = adc_channel_1[0];
      modbus_write_holding_register(&modbus, 4, last_state_adc_ch1);
    }

    // printf("Coil[0] = %d\n", modbus_read_coil(&modbus, 0));
    // printf("Input Status[0] = %d\n", modbus_read_input_status(&modbus, 0));
    // printf("Holding Register[0] = %d\n", modbus_read_holding_register(&modbus, 0));
    // printf("Input Register[0] = %d\n", modbus_read_input_register(&modbus, 0));

    sleep_ms(100);
  }
}




// FUNCIOUNOU PERFEITAMENTE!!!
/*
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/dma.h"

// Configurações do ADC
#define ADC_CHANNELS 2     // Número de canais ADC
#define BUFFER_SIZE 256    // Número de amostras no buffer (deve ser múltiplo do número de canais)

// Buffer para armazenar leituras do ADC
uint16_t adc_buffer[BUFFER_SIZE];

// Canal DMA
int dma_chan;

void adc_dma_handler()
{
  // Limpa a flag da interrupção do DMA
  dma_hw->ints0 = 1u << dma_chan;

  // Reinicia a transferência do DMA
  dma_channel_transfer_to_buffer_now(dma_chan, adc_buffer, BUFFER_SIZE);
}

void adc_dma_init()
{
  // Inicializa o ADC
  adc_init();

  // Configura os pinos como entradas analógicas
  adc_gpio_init(26);  // Canal 0 (GP26)
  adc_gpio_init(27);  // Canal 1 (GP27)

  // Configura o modo round-robin para alternar entre os canais 0 e 1
  adc_set_round_robin((1 << 0) | (1 << 1)); // Canais 0 e 1

  // Configura FIFO do ADC
  adc_fifo_setup(
      true,  // Ativa FIFO
      true,  // Aciona DMA quando há dados
      1,     // Solicitação de DMA a cada amostra
      false, // Sem multiplicação de valores
      false  // FIFO em 16 bits (incluindo os 4 bits de canal)
  );

  // Inicia conversão contínua
  adc_run(true);

  // Configuração do DMA
  dma_chan = dma_claim_unused_channel(true);
  dma_channel_config c = dma_channel_get_default_config(dma_chan);

  channel_config_set_transfer_data_size(&c, DMA_SIZE_16);
  channel_config_set_read_increment(&c, false);
  channel_config_set_write_increment(&c, true);
  channel_config_set_dreq(&c, DREQ_ADC);

  // Configura DMA para transferir dados do ADC para o buffer
  dma_channel_configure(
      dma_chan, &c,
      adc_buffer,    // Destino: buffer
      &adc_hw->fifo, // Fonte: FIFO do ADC
      BUFFER_SIZE,   // Tamanho da transferência
      false          // Não inicia ainda
  );

  // Configura interrupção do DMA
  dma_channel_set_irq0_enabled(dma_chan, true);
  irq_set_exclusive_handler(DMA_IRQ_0, adc_dma_handler);
  irq_set_enabled(DMA_IRQ_0, true);

  // Inicia a primeira transferência
  dma_channel_transfer_to_buffer_now(dma_chan, adc_buffer, BUFFER_SIZE);
}

int main()
{
  stdio_init_all(); // Inicializa comunicação serial
  adc_dma_init();   // Inicializa ADC e DMA

  while (1)
  {
    printf("Leituras do ADC:\n");
    for (int i = 0; i < 10; i++)
    { // Exibir apenas os primeiros 10 valores
      uint16_t raw = adc_buffer[i];
      uint8_t channel = i % ADC_CHANNELS; // Determina o canal com base no índice
      uint16_t value = raw & 0xFFF;       // Extrai o valor de 12 bits
      printf("Canal %d: %d (%.2f V)\n", channel, value, value * 3.3 / 4095);
    }
    printf("\n");

    sleep_ms(1000);
  }

  return 0;
}
  */

// LEITURA DE DOIS CANAIS
// FUNCIONOU!!
/*
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/dma.h"

// Configurações do ADC
#define ADC_CHANNELS 2  // Número de canais ADC
#define BUFFER_SIZE 256 // Número de amostras no buffer (deve ser múltiplo do número de canais)

// Buffer para armazenar leituras do ADC
uint16_t adc_buffer[BUFFER_SIZE];

// Canal DMA
int dma_chan;

void adc_dma_handler()
{
  // Limpa a flag da interrupção do DMA
  dma_hw->ints0 = 1u << dma_chan;

  // Reinicia a transferência do DMA
  dma_channel_transfer_to_buffer_now(dma_chan, adc_buffer, BUFFER_SIZE);
}

void adc_dma_init()
{
  // Inicializa o ADC
  adc_init();

  // Configura os pinos como entradas analógicas
  adc_gpio_init(26); // Canal 0 (GP26)
  adc_gpio_init(27); // Canal 1 (GP27)

  // Configura o modo round-robin para alternar entre os canais 0 e 1
  adc_set_round_robin((1 << 0) | (1 << 1));

  // Configura FIFO do ADC
  adc_fifo_setup(
      true,  // Ativa FIFO
      true,  // Aciona DMA quando há dados
      1,     // Solicitação de DMA a cada amostra
      false, // Sem multiplicação de valores
      true   // FIFO em 12 bits
  );

  // Inicia conversão contínua
  adc_run(true);

  // Configuração do DMA
  dma_chan = dma_claim_unused_channel(true);
  dma_channel_config c = dma_channel_get_default_config(dma_chan);

  channel_config_set_transfer_data_size(&c, DMA_SIZE_16);
  channel_config_set_read_increment(&c, false);
  channel_config_set_write_increment(&c, true);
  channel_config_set_dreq(&c, DREQ_ADC);

  // Configura DMA para transferir dados do ADC para o buffer
  dma_channel_configure(
      dma_chan, &c,
      adc_buffer,    // Destino: buffer
      &adc_hw->fifo, // Fonte: FIFO do ADC
      BUFFER_SIZE,   // Tamanho da transferência
      false          // Não inicia ainda
  );

  // Configura interrupção do DMA
  dma_channel_set_irq0_enabled(dma_chan, true);
  irq_set_exclusive_handler(DMA_IRQ_0, adc_dma_handler);
  irq_set_enabled(DMA_IRQ_0, true);

  // Inicia a primeira transferência
  dma_channel_transfer_to_buffer_now(dma_chan, adc_buffer, BUFFER_SIZE);
}

int main()
{
  stdio_init_all(); // Inicializa comunicação serial
  adc_dma_init();   // Inicializa ADC e DMA

  while (1)
  {
    printf("Leituras do ADC:\n");
    for (int i = 0; i < 10; i++)
    { // Exibir apenas os primeiros 10 valores
      uint16_t raw = adc_buffer[i];
      uint8_t channel = (i % ADC_CHANNELS); // Determina o canal
      uint16_t value = raw & 0xFFF;         // Extrai o valor de 12 bits
      printf("Canal %d: %d (%.2f V)\n", channel, value, value * 3.3 / 4095);
    }
    printf("\n");

    sleep_ms(1000);
  }

  return 0;
}

*/

// LEITURA DE APENAS UM CANAL
// FUNCIONOU!!!
/*
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/dma.h"

// Configurações do ADC
#define ADC_INPUT 0     // Canal do ADC (GPIO26)
#define BUFFER_SIZE 256 // Número de amostras no buffer

// Buffer para armazenar leituras do ADC
uint16_t adc_buffer[BUFFER_SIZE];

// Canal DMA
int dma_chan;

void adc_dma_handler()
{
  // Limpa a flag da interrupção do DMA
  dma_hw->ints0 = 1u << dma_chan;

  // Reinicia a transferência do DMA
  dma_channel_transfer_to_buffer_now(dma_chan, adc_buffer, BUFFER_SIZE);
}

void adc_dma_init()
{
  // Inicializa o ADC e configura o canal
  adc_init();
  adc_gpio_init(26 + ADC_INPUT);
  adc_select_input(ADC_INPUT);

  // Configura FIFO do ADC
  adc_fifo_setup(
      true,  // Ativa FIFO
      true,  // Aciona DMA quando há dados
      1,     // Solicitação de DMA a cada amostra
      false, // Sem multiplicação de valores
      true   // FIFO em 12 bits
  );

  // Inicia conversão contínua
  adc_run(true);

  // Configuração do DMA
  dma_chan = dma_claim_unused_channel(true);
  dma_channel_config c = dma_channel_get_default_config(dma_chan);

  channel_config_set_transfer_data_size(&c, DMA_SIZE_16);
  channel_config_set_read_increment(&c, false);
  channel_config_set_write_increment(&c, true);
  channel_config_set_dreq(&c, DREQ_ADC);

  // Configura DMA para transferir dados do ADC para o buffer
  dma_channel_configure(
      dma_chan, &c,
      adc_buffer,    // Destino: buffer
      &adc_hw->fifo, // Fonte: FIFO do ADC
      BUFFER_SIZE,   // Tamanho da transferência
      false          // Não inicia ainda
  );

  // Configura interrupção do DMA
  dma_channel_set_irq0_enabled(dma_chan, true);
  irq_set_exclusive_handler(DMA_IRQ_0, adc_dma_handler);
  irq_set_enabled(DMA_IRQ_0, true);

  // Inicia a primeira transferência
  dma_channel_transfer_to_buffer_now(dma_chan, adc_buffer, BUFFER_SIZE);
}

int main()
{
  stdio_init_all(); // Inicializa comunicação serial
  adc_dma_init();   // Inicializa ADC e DMA

  while (1)
  {
    printf("Leituras do ADC: ");
    for (int i = 0; i < 10; i++)
    { // Exibir apenas os primeiros 10 valores
      printf("%d ", adc_buffer[i]);
    }
    printf("\n");

    sleep_ms(1000);
  }

  return 0;
}
*/

/*
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/sync.h"

#include "my_std_debug.h"
#include "my_configs.h"
#include "my_wifi.h"
#include "my_gpio.h"
#include "my_dma.h"
#include "my_modbus_master.h"


int main()
{
  stdio_init_all();

  my_gpio_t btn = {
    .dir      = GPIO_IN,
    .pin      = MY_GPIO_IN_BTN_A,
    .res_pull = GPIO_RES_PULL_UP
  };

  my_modbus_master_t modbus = {
    .server_ip   = MY_CONFIGS_IP_MODBUS,
    .server_port = MY_CONFIGS_PORT_MODBUS,
    .slave_id    = MY_CONFIGS_SID_MODBUS
  };

  int adc_channel_init[] = {MY_GPIO_ADC_IN_CH_JOYSTICK_X, MY_GPIO_ADC_IN_CH_JOYSTICK_Y};

  my_gpio_init(&btn);
  my_dma_init(adc_channel_init, sizeof(adc_channel_init));

  sleep_ms(2000);

  while ( my_wifi_init(MY_CONFIGS_WIFI_SSID, MY_CONFIGS_WIFI_PASSWORD) != MY_STD_RET_OK)
  {
    sleep_ms(100);
  }

  MY_STD_DEBUG_ERROR_CHECK( modbus_connect(&modbus) );

  while (1)
  {
    if( !gpio_get(btn.pin) )
    {
      modbus_write_coil(&modbus, 0, 1);
    }
    else
    {
      modbus_write_coil(&modbus, 0, 0);
    }

    modbus_write_holding_register(&modbus, 3, my_adc_get_digital_value(MY_GPIO_ADC_IN_CH_JOYSTICK_X));
    modbus_write_holding_register(&modbus, 4, my_adc_get_digital_value(MY_GPIO_ADC_IN_CH_JOYSTICK_Y));

    printf("Coil[0] = %d\n", modbus_read_coil(&modbus, 0));
    printf("Input Status[0] = %d\n", modbus_read_input_status(&modbus, 0));
    printf("Holding Register[0] = %d\n", modbus_read_holding_register(&modbus, 0));
    printf("Input Register[0] = %d\n", modbus_read_input_register(&modbus, 0));
  }
}
*/

/*
void enter_sleep_mode()
{
  // printf("Entrando no modo sleep...\n");

  // Reduza o consumo desativando periféricos não usados aqui (opcional)

  __wfi(); // Entra em modo de espera
  // printf("Acordado do modo sleep!\n");
}
*/