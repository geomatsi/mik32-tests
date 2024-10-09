#include "scr1_csr_encoding.h"
#include "xprintf.h"
#include "csr.h"

#include "mik32_hal_irq.h"

#include "elbear_init.h"
#include "dbtr.h"

#define ILL_INSN (0x00000000)

USART_HandleTypeDef husart0;

void GPIO_Init(void)
{
	GPIO_InitTypeDef init = {0};

	__HAL_PCC_GPIO_2_CLK_ENABLE();
	__HAL_PCC_GPIO_IRQ_CLK_ENABLE();

	/* LED: pin 2_7 */
	init.Pin = GPIO_PIN_7;
	init.Mode = HAL_GPIO_MODE_GPIO_OUTPUT;
	init.Pull = HAL_GPIO_PULL_NONE;

	HAL_GPIO_Init(GPIO_2, &init);

	/* User button: pin 2_6 */
	init.Pin = GPIO_PIN_6;
	init.Mode = HAL_GPIO_MODE_GPIO_INPUT;
	init.Pull = HAL_GPIO_PULL_NONE;

	HAL_GPIO_Init(GPIO_2, &init);
	HAL_GPIO_InitInterruptLine(GPIO_MUX_PORT2_6_LINE_2, GPIO_INT_MODE_CHANGE);
}

void EPIC_Init(void)
{
    __HAL_PCC_EPIC_CLK_ENABLE();

    HAL_EPIC_MaskLevelSet(HAL_EPIC_GPIO_IRQ_MASK);
    HAL_IRQ_EnableInterrupts();
}

void trap_handler(void) __attribute__((interrupt("machine"))) __attribute__((section(".trap")));
void trap_handler(void)
{
	unsigned long mcause, mtval, mepc;
	unsigned long t1 = 0;
	int step_over = 0;

	mcause = read_csr(mcause);
	mtval = read_csr(mtval);
	mepc = read_csr(mepc);

	xprintf("%s: mcause(0x%lx) mtval(0x%lx) mepc(0x%lx)\n",
		__func__, mcause, mtval, mepc);

	switch (mcause) {
		case MCAUSE_MACHINE_EXTERNAL_INTERRUPT:
			if (EPIC_CHECK_GPIO_IRQ()) {
				if (HAL_GPIO_LineInterruptState(GPIO_LINE_2)) {
					xprintf("%s: interrupt: button level %d...\n", __func__,
						HAL_GPIO_LinePinState(GPIO_LINE_2));
				}
				HAL_GPIO_ClearInterrupts();
			}
			HAL_EPIC_Clear(0xFFFFFFFF);
			break;
		case MCAUSE_ILLEGAL_INSTRUCTION:
			xprintf("%s: exception: illegal instruction...\n", __func__);
			step_over = 1;
			break;
		default:
			xprintf("%s: unexpected exception: 0x%lx...\n",
				__func__, mcause);
			goto error;
	}

	/* Note:
	 * - for simplicity we assume that triggering insn is 4-bytes
	 * - proper code should decode insn and figure out its size
	 */
	if (step_over) {
		__asm__ volatile ( /* step */
			"csrr %0, mepc\n\t"
			"addi %0, %0, 4\n\t"
			"csrw	mepc, %0\n\t"
			: "+r"(t1) : : );
	}

	return;

error:
	xprintf("%s: error condition in trap handler...\n", __func__);

	while (1) {
		__asm__ volatile ("wfi");
	}

	return;
}

void main(void)
{
	SystemClock_Config();
	USART_Init(&husart0);
	GPIO_Init();
	EPIC_Init();

	while (1) {
		__asm__ volatile (".word %0" : : "i"(ILL_INSN) : );
		HAL_GPIO_TogglePin(GPIO_2, GPIO_PIN_7);
		HAL_DelayMs(5000);
	}
}
