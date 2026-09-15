#ifndef HARDWARE_GPIO_H
#define HARDWARE_GPIO_H
// HOST TEST STUB
typedef unsigned int uint;
enum gpio_function_t { GPIO_FUNC_SIO = 5, GPIO_FUNC_USB = 9, GPIO_FUNC_NULL = 0x1f };
void gpio_init(uint gpio);
void gpio_set_function(uint gpio, gpio_function_t fn);
void gpio_set_dir(uint gpio, bool out);
void gpio_pull_down(uint gpio);
bool gpio_get(uint gpio);
#endif
