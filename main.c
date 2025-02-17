#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/sync.h"

#include "my_std_debug.h"
#include "my_configs.h"
#include "my_wifi.h"
#include "my_gpio.h"
#include "my_adc.h"
#include "my_dma.h"
#include "my_modbus_master.h"

// histery para atenuar os ruidos
#define HYSTERY_ADC_VALUE 50UL

enum joystick_enum
{
  JOYSTICK_AD_X = 0,
  JOYSTICK_AD_Y,
  JOYSTICK_AD_LENGTH
};

enum btn_enum 
{
  BTN_LIGA_PLANTA = 0,
  BTN_DESLIGA_PLANTA,
  BTN_LENGTH
};

enum led_enum 
{
  LED_VERDE = 0,
  LED_VERMELHO,
  LED_LENGTH
};

enum modbus_addr_input_status_enum
{
  MODBUS_ADDR_INPUT_STATUS_LED_VERDE    = 0,
  MODBUS_ADDR_INPUT_STATUS_LED_VERMELHO = 1,
};

enum modbus_addr_holding_register_enum
{
  MODBUS_ADDR_HOLDING_REGISTER_ESTEIRA = 4,
};

int main()
{
  stdio_init_all();

  sleep_ms(500);

  my_gpio_t btn[BTN_LENGTH] = {
    [BTN_LIGA_PLANTA] = {
      .dir = GPIO_IN,
      .pin = MY_GPIO_IN_BTN_A,
      .res_pull = GPIO_RES_PULL_UP
    },
    [BTN_DESLIGA_PLANTA] = {
      .dir = GPIO_IN,
      .pin = MY_GPIO_IN_BTN_B,
      .res_pull = GPIO_RES_PULL_UP
    }
  };

  my_gpio_t led[LED_LENGTH] = {
    [LED_VERDE] = {
      .dir = GPIO_OUT,
      .pin = MY_GPIO_OUT_LED_VERDE
    },
    [LED_VERMELHO] = {
      .dir = GPIO_OUT,
      .pin = MY_GPIO_OUT_LED_VERMELHO
    }
  };

  my_modbus_master_t modbus = {
    .server_ip = MY_CONFIGS_IP_MODBUS,
    .server_port = MY_CONFIGS_PORT_MODBUS,
    .slave_id = MY_CONFIGS_SID_MODBUS
  };

  my_adc_t adc_channel[JOYSTICK_AD_LENGTH] = {
    [JOYSTICK_AD_X] = {
      .pin = MY_GPIO_ADC_IN_CH_JOYSTICK_X
    },
    [JOYSTICK_AD_Y] = {
      .pin = MY_GPIO_ADC_IN_CH_JOYSTICK_Y
    }
  };

  bool planta_ligada = false;

  while (my_wifi_init(MY_CONFIGS_WIFI_SSID, MY_CONFIGS_WIFI_PASSWORD) != MY_STD_RET_OK)
  {
    sleep_ms(100);
  }

  // iniciliaza pinos de entrada
  for(register int i = 0; i < BTN_LENGTH; i++)
    my_gpio_init(&btn[i]);

  // inicializa pinos de saída
  for(register int i = 0; i < LED_LENGTH; i++) 
    my_gpio_init(&led[i]);

  // define estado dos leds em funcao da flag planta_ligada
  if(planta_ligada)
  {
    gpio_put(led[LED_VERDE].pin, true);
    gpio_put(led[LED_VERMELHO].pin, false);
  }
  else
  {
    gpio_put(led[LED_VERDE].pin, false);
    gpio_put(led[LED_VERMELHO].pin, true);
  }
  
  // inicializa dma para pegar amostras do adc continuamente
  my_dma_init((int[]){adc_channel[JOYSTICK_AD_X].pin, adc_channel[JOYSTICK_AD_Y].pin}, sizeof(adc_channel) / sizeof(adc_channel[0]));

  // conecta no servidor modbus
  MY_STD_DEBUG_ERROR_CHECK(modbus_connect(&modbus));

  // obtem os atuais estados dos botoes fisicos
  for(register int i = 0; i < BTN_LENGTH; i++)
    btn[i].last_state = btn[i].current_state = gpio_get(btn[i].pin);

  // obtem as primeiras amostras do adc coletadas pelo dma
  my_dma_get_value(adc_channel[JOYSTICK_AD_X].pin, 1, &adc_channel[JOYSTICK_AD_X].current_value);
  my_dma_get_value(adc_channel[JOYSTICK_AD_Y].pin, 1, &adc_channel[JOYSTICK_AD_Y].current_value);

  // atualiza os ultimos valores da estrutura adc_channel
  adc_channel[JOYSTICK_AD_X].last_value = adc_channel[JOYSTICK_AD_X].current_value;
  adc_channel[JOYSTICK_AD_Y].last_value = adc_channel[JOYSTICK_AD_Y].current_value;

  while (1)
  {
    // verifica se o nível do botão alterou, se sim então liga a planta virtual
    if ((btn[BTN_DESLIGA_PLANTA].current_state = gpio_get(btn[BTN_DESLIGA_PLANTA].pin)) != btn[BTN_DESLIGA_PLANTA].last_state)
    {
      btn[BTN_DESLIGA_PLANTA].last_state = btn[BTN_DESLIGA_PLANTA].current_state;

      planta_ligada = false;

      // envia a leitura para a rede
      modbus_write_coil(&modbus, MODBUS_ADDR_INPUT_STATUS_LED_VERDE, false);           // desliga led verde no factory io
      modbus_write_coil(&modbus, MODBUS_ADDR_INPUT_STATUS_LED_VERMELHO, true);         // liga led vermelho no factory io
      modbus_write_holding_register(&modbus, MODBUS_ADDR_HOLDING_REGISTER_ESTEIRA, 0); // desliga esteira no factory io
    }
    // verifica se o nível do botão alterou, se sim então liga a planta virtual
    else if ((btn[BTN_LIGA_PLANTA].current_state = gpio_get(btn[BTN_LIGA_PLANTA].pin)) != btn[BTN_LIGA_PLANTA].last_state)
    {
      // atualiza o ultimo estado de transicao do botao
      btn[BTN_LIGA_PLANTA].last_state = btn[BTN_LIGA_PLANTA].current_state;

      // atualiza flag
      planta_ligada = true; 

      // envia os dados para a rede
      modbus_write_coil(&modbus, MODBUS_ADDR_INPUT_STATUS_LED_VERDE, true);     // liga led verde no factory io
      modbus_write_coil(&modbus, MODBUS_ADDR_INPUT_STATUS_LED_VERMELHO, false); // desliga led vermelho no factory io
    }

    if(planta_ligada)
    {
      // obtem valor da amostragem direto da memoria na qual o dma disponibiliza continuamente
      my_dma_get_value(adc_channel[JOYSTICK_AD_Y].pin, 1, &adc_channel[JOYSTICK_AD_Y].current_value);

      // verifica se o joystick se movimentou (hystery foi utilizada para atenuar os ruidos do canal adc)
      if (adc_channel[JOYSTICK_AD_Y].last_value < adc_channel[JOYSTICK_AD_Y].current_value - HYSTERY_ADC_VALUE || adc_channel[JOYSTICK_AD_Y].last_value > adc_channel[JOYSTICK_AD_Y].current_value + HYSTERY_ADC_VALUE)
      {
        // atualiza a umtimo leitura valida
        adc_channel[JOYSTICK_AD_Y].last_value = adc_channel[JOYSTICK_AD_Y].current_value;

        if (adc_channel[JOYSTICK_AD_Y].last_value <= 1850)
        {
          modbus_write_holding_register(&modbus, MODBUS_ADDR_HOLDING_REGISTER_ESTEIRA, adc_channel[JOYSTICK_AD_Y].last_value * 0.270);
        }
        else if (adc_channel[JOYSTICK_AD_Y].last_value >= 1870)
        {
          modbus_write_holding_register(&modbus, MODBUS_ADDR_HOLDING_REGISTER_ESTEIRA, adc_channel[JOYSTICK_AD_Y].last_value * 0.244);
        }
      }
    }
  }
}