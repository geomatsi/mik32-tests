#include "elbear_init.h"

void main(void)
{
	SystemClock_Config();
	GPIO_LED_Init();

	while (1) {
		HAL_GPIO_TogglePin(GPIO_2, GPIO_PIN_7);
		HAL_DelayMs(100);
	}
}
