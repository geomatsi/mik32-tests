#include "elbear_init.h"

#define ILL_INSN (0x00000000)

void trap_handler(void) __attribute__((interrupt("machine"))) __attribute__((section(".trap")));
void trap_handler(void)
{
	unsigned long mcause;
	unsigned long t1 = 0;
	int step_over = 0;

	__asm__ volatile ("csrr %0, mcause\n\t"
		: "=r" (mcause) : : );

	switch (mcause) {
		case 2: /* illegal insn */
			HAL_GPIO_WritePin(GPIO_2, GPIO_PIN_7, GPIO_PIN_HIGH);
			HAL_DelayMs(500);
			HAL_GPIO_WritePin(GPIO_2, GPIO_PIN_7, GPIO_PIN_LOW);
			HAL_DelayMs(500);

			step_over = 1;
			break;
		case 11: /* ecall */

			HAL_GPIO_WritePin(GPIO_2, GPIO_PIN_7, GPIO_PIN_HIGH);
			HAL_DelayMs(1000);
			HAL_GPIO_WritePin(GPIO_2, GPIO_PIN_7, GPIO_PIN_LOW);
			HAL_DelayMs(1000);

			step_over = 1;
			break;
		default:
			goto failure;
	}

	if (step_over) {
		__asm__ volatile ( /* step */
			"csrr %0, mepc\n\t"
			"addi %0, %0, 4\n\t"
			"csrw	mepc, %0\n\t"
			: "+r"(t1) : : );
	}

	return;

failure:
	while (1) {
		HAL_GPIO_WritePin(GPIO_2, GPIO_PIN_7, GPIO_PIN_HIGH);
		HAL_DelayMs(50);
		HAL_GPIO_WritePin(GPIO_2, GPIO_PIN_7, GPIO_PIN_LOW);
		HAL_DelayMs(50);
	}

	return;
}

void main(void)
{
	int c;

	SystemClock_Config();
	GPIO_LED_Init();

	while (1) {
		for(c = 0; c < 5; c++) {
			/* expelliarmus */
			__asm__ volatile ("ecall");
		}

		for(c = 0; c < 5; c++) {
			/* stupefy */
			__asm__ volatile (".word %0" : : "i"(ILL_INSN) : );
		}
	}
}
