#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/sync.h"

#include "my_std_debug.h"
#include "my_configs.h"
#include "my_wifi.h"
#include "my_gpio.h"
#include "my_adc.h"
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

  my_gpio_init(&btn);
  my_adc_init(MY_GPIO_ADC_IN_INIT);

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

/*
void enter_sleep_mode();

int main()
{

  stdio_init_all();

  while (my_wifi_init(MY_CONFIGS_WIFI_SSID, MY_CONFIGS_WIFI_PASSWORD) != MY_STD_RET_OK);


  // modbus_init("192.168.15.3");
  modbus_connect("192.168.15.3", 502);

  sleep_ms(2000); // Espera a conexão estabelecer

  // Envia um comando Modbus para ler registros
  printf("Enviando comando Modbus TCP...\n");

  bool status = false;

  while (true)
  {
    // modbus_send_read_holding_registers(1, 1, 0, 1);
    modbus_write_coil(0x0000, status);

    modbus_read_coils(0x0000, 1);  // Reenvia a leitura a cada 5s

    status = !status;

    sleep_ms(1000);

  }
}







void enter_sleep_mode()
{
  // printf("Entrando no modo sleep...\n");

  // Reduza o consumo desativando periféricos não usados aqui (opcional)

  __wfi(); // Entra em modo de espera
  // printf("Acordado do modo sleep!\n");
}
*/