#include "my_i2c.h"

my_std_ret_t my_i2c_init(i2c_inst_t *i2c, uint pin_sda, uint pin_scl, uint freq)
{
    i2c_init(i2c, freq);
    gpio_set_function(pin_scl, GPIO_FUNC_I2C);
    gpio_set_function(pin_sda, GPIO_FUNC_I2C);
    gpio_pull_up(pin_scl);
    gpio_pull_up(pin_sda);

    return MY_STD_RET_OK;
}
