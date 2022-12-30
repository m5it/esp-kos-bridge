//
#include "driver/gpio.h"
#include "driver/ledc.h"
/*#define GPIO_ENABLE_REG 0x3FF44020
#define GPIO_OUT_REG 0x0004
//
int t3ch_gpio_state(int gpio) {
	gpio_num_t pin = (gpio_num_t)(gpio & 0x1F);
	int state = 0;

	if (GPIO_REG_READ(GPIO_ENABLE_REG) & BIT(pin))
	{
		return (GPIO_REG_READ(GPIO_OUT_REG)  >> pin) & 1U;
	}
	return -1;
}*/
