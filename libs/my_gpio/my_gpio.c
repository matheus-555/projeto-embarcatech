#include "my_gpio.h"

// callback das rotinas de condiguracao do resistor de pull
void (*func_gpio_res_callback[GPIO_RES_LENGTH - 1])(unsigned int) = {
  [GPIO_RES_PULL_DOWN] = &gpio_pull_down,
  [GPIO_RES_PULL_UP] = &gpio_pull_up
};



void my_gpio_init(my_gpio_t *gpio)
{
  gpio_init(gpio->pin);
  gpio_set_dir(gpio->pin, gpio->dir);

  if (gpio->dir == GPIO_OUT                   ||
      gpio->res_pull == GPIO_RES_PULL_DISABLE ||
      gpio->res_pull >= GPIO_RES_LENGTH )
    return;

  func_gpio_res_callback[gpio->res_pull](gpio->pin);

  gpio->press_last_time = 0;
  gpio->press_counter = 0;
  gpio->pressed_flag = false;
}