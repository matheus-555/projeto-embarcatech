#ifndef __MY_ADC_H__
#define __MY_ADC_H__


#include "my_std_ret.h"
#include "my_gpio.h"


my_std_ret_t my_adc_init(uint gpio_adc_init_pin);
uint my_adc_get_digital_value(uint adc_ch_pin);



#endif