#ifndef __MY_GPIO_H__
#define __MY_GPIO_H__

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

//==========================================================================
// MAPEAMENTO DA BITDOGLAB
//==========================================================================

// --- pinos de saida digital
#define MY_GPIO_OUT_LED_VERMELHO 13UL // OK
#define MY_GPIO_OUT_LED_VERDE    11UL // OK
#define MY_GPIO_OUT_LED_AZUL     12UL // OK
#define MY_GPIO_OUT_BUZZER_A     21UL // OK
#define MY_GPIO_OUT_BUZZER_B     10UL // OK

// --- pinos de entrada digital
#define MY_GPIO_IN_BTN_A          5UL  // OK (Habilitar Pullup)
#define MY_GPIO_IN_BTN_B          6UL  // OK (Habilitar Pullup)
#define MY_GPIO_IN_JOYSTICK_BTN   22UL // OK (Habilitar Pullup)

// --- pino matriz de led
#define MY_GPIO_OUT_MATRIX_LED    7UL

// --- pinos de entrada analogica


#define MY_GPIO_ADC_IN_INIT        26UL   // OK (Colocar no adc_gpio_init())

#define MY_GPIO_ADC_IN_CH_MICROFONE   2UL // OK (Colocar no adc_select_input())
#define MY_GPIO_ADC_IN_CH_JOYSTICK_Y  0UL // OK (Colocar no adc_select_input())
#define MY_GPIO_ADC_IN_CH_JOYSTICK_X  1UL // OK (Colocar no adc_select_input())


// --- pinos i2c
#define MY_GPIO_I2C1               i2c1      // OK
#define MY_GPIO_I2C1_FREQ          400000UL  // Ok
#define MY_GPIO_I2C1_SDA           14UL      // OK
#define MY_GPIO_I2C1_SCL           15UL      // OK



//==========================================================================
// TYPEDEFS PUBLICOS
//==========================================================================

// --- tipo de direcao do pino
typedef uint8_t gpio_dir_t;

// --- enum para enumerar os tipos de configuracao dos resistores de pull
typedef enum {
  GPIO_RES_PULL_DOWN = 0,
  GPIO_RES_PULL_UP,
  GPIO_RES_PULL_DISABLE,
  GPIO_RES_LENGTH
} gpio_res_pull_t;

// --- estrutura para controle de gpio
typedef struct {
  uint pin;
  gpio_dir_t dir;
  gpio_res_pull_t res_pull;
  absolute_time_t press_last_time;
  uint press_counter;
  bool pressed_flag;
} my_gpio_t;


//==========================================================================
// PROTOTIPOS DE FUNCOES PUBLICAS
//==========================================================================

// inicializa o pino em funcao dos parametros da estrutura
void my_gpio_init(my_gpio_t *gpio);

#endif