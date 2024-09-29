#include "elbear_init.h"
#include "xprintf.h"

USART_HandleTypeDef husart0;

void main(void)
{
	unsigned long count = 0;

	SystemClock_Config();
	GPIO_LED_Init();
	USART_Init(&husart0);

	HAL_USART_Print(&husart0, "HAL: start...\n", USART_TIMEOUT_DEFAULT);
	xprintf("SHARED: start...\n");

	while (1) {
		HAL_GPIO_TogglePin(GPIO_2, GPIO_PIN_7);
		HAL_DelayMs(500);

		xprintf("count: %lu\n", count++);
	}
}
