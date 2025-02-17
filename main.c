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


void enter_sleep_mode();

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


void enter_sleep_mode()
{
  // printf("Entrando no modo sleep...\n");

  // Reduza o consumo desativando periféricos não usados aqui (opcional)

  __wfi(); // Entra em modo de espera
  // printf("Acordado do modo sleep!\n");
}