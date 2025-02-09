#ifndef __MY_I2C_H__
#define __MY_I2C_H__


#include "my_std_ret.h"
#include "my_gpio.h"

my_std_ret_t my_i2c_init(i2c_inst_t *i2c, uint pin_sda, uint pin_scl, uint freq);


#endif