#ifndef __MY_ADC_H__
#define __MY_ADC_H__


#include "my_std_ret.h"
#include "my_gpio.h"

typedef struct 
{
    int pin;
    int current_value;
    int last_value;
} my_adc_t;


my_std_ret_t my_adc_init(uint gpio_adc_init_pin);
uint my_adc_get_digital_value(uint adc_ch_pin);



#endif