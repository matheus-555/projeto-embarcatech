#include "my_adc.h"
#include "hardware/adc.h"

my_std_ret_t my_adc_init(uint gpio_adc_init_pin)
{
    adc_init();

    adc_gpio_init(gpio_adc_init_pin);

    return MY_STD_RET_OK;
}


inline uint my_adc_get_digital_value(uint adc_ch_pin)
{
    adc_select_input(adc_ch_pin); // 0 corresponde ao pino GP26

    return adc_read();
}
