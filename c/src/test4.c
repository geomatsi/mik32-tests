#include "elbear_init.h"
#include "xprintf.h"
#include "dbtr.h"
#include "csr.h"

USART_HandleTypeDef husart0;
int ret;

void trap_handler(void) __attribute__((interrupt("machine"))) __attribute__((section(".trap")));
void trap_handler(void)
{
	unsigned long mcause = read_csr(mcause);
	unsigned long t1 = 0;
	int step_over = 0;

	/* reset trap status */
	ret = 0;

	switch (mcause) {
		case 2: /* illegal insn */
			step_over = 1;
			ret = 1;
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
	xprintf("%s: unexpected exception: 0x%lx...\n",
		__func__, mcause);

	HAL_GPIO_WritePin(GPIO_2, GPIO_PIN_7, GPIO_PIN_HIGH);

	while (1) {
		__asm__ volatile ("wfi");
	}

	return;
}

void main(void)
{
	unsigned long c, v;

	SystemClock_Config();
	USART_Init(&husart0);
	GPIO_LED_Init();

	xprintf("%s: enumerate triggers...\n", __func__);

	/* DBTR enumerate procedure */

	for (c = 0; ; c++) {
		write_csr(tselect, c);
		if (ret) {
			xprintf("%s(%d): no %s triggers implemented...\n",
				__func__, __LINE__, c ? "more" : "");
			break;
		}

		v = read_csr(tselect);
		if (v != c) {
			xprintf("%s(%d): no %s triggers implemented...\n",
				__func__, __LINE__, c ? "more" : "");
			break;
		}

		v = read_csr(tinfo);
		if (ret) {
			v = read_csr(tdata1);
			if (ret || (v == 0)) {
				xprintf("%s(%d): no %s triggers implemented...\n",
					__func__, __LINE__, c ? "more" : "");
				break;
			}
		} else {
			if (TINFO_DBTR_NOEXIST(v)) {
				xprintf("%s(%d): no %s triggers implemented...\n",
					__func__, __LINE__, c ? "more" : "");
				break;
			}
		}

		xprintf("%s: trigger %u supported types: [ %s%s]\n", __func__, c,
			TINFO_DBTR_MCONTROL(v) ? "MCONTROL " : "",
			TINFO_DBTR_ICOUNT(v) ? "ICOUNT " : "");

		/* check mcontrol trigger properties assuming it is WARL */
		if (TINFO_DBTR_MCONTROL(v)) {
			write_csr(tdata1, 0x0);
			if (ret) {
				xprintf("%s: failed to zero tdata1 for trigger %lu...\n",
					__func__, c);
				continue;
			}

			write_csr(tdata2, 0x0);
			if (ret) {
				xprintf("%s: failed to zero tdata2 for trigger %lu...\n",
					__func__, c);
				continue;
			}

			v = TDATA1_MCONTROL_LOAD  | TDATA1_MCONTROL_STORE | TDATA1_MCONTROL_EXEC   |
				TDATA1_MCONTROL_MODE_M | TDATA1_MCONTROL_CHAIN;

			xprintf("%s: trigger %lu: write 0x%lx to tdata1\n", __func__, c, v);
			write_csr(tdata1, v);
			if (ret) {
				xprintf("%s: failed to set tdata1 for trigger %lu...\n",
					__func__, c);
				continue;
			}

			v = read_csr(tdata1);
			xprintf("%s: trigger %lu: read 0x%lx from tdata1\n", __func__, c, v);
			xprintf("%s: trigger %lu: [%s%s%s%s%s ]\n", __func__, c,
				(v & TDATA1_MCONTROL_LOAD)  ? " LOAD"  : "",
				(v & TDATA1_MCONTROL_STORE) ? " STORE" : "",
				(v & TDATA1_MCONTROL_EXEC)  ? " EXEC"  : "",
				(v & TDATA1_MCONTROL_MODE_M)  ? " MODE_M"  : "",
				(v & TDATA1_MCONTROL_CHAIN) ? " CHAIN" : "");
		}

		/* check icount trigger properties assuming it is WARL */
		if (TINFO_DBTR_ICOUNT(v)) {
			v = TDATA1_ICOUNT_MODE_M | TDATA1_ICOUNT_COUNT_MAX;
			xprintf("%s: trigger %lu: write 0x%lx to tdata1\n", __func__, c, v);
			write_csr(tdata1, v);
			if (ret) {
				xprintf("%s: failed to set tdata1 for trigger %lu...\n",
					__func__, c);
				continue;
			}

			/* deactivate after 3 instructions including previous comparison */
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			v = read_csr(tdata1);
			write_csr(tdata1, 0x0);
			xprintf("%s: trigger %lu: read 0x%lx from tdata1\n", __func__, c, v);
			xprintf("%s: trigger %lu: [%s %lu]\n", __func__, c,
				(v & TDATA1_ICOUNT_MODE_M)  ? " MODE_M"  : "",
				TDATA1_ICOUNT_COUNT_R(v));
		}
	}

	while (1) {
		HAL_GPIO_WritePin(GPIO_2, GPIO_PIN_7, GPIO_PIN_HIGH);
		HAL_DelayMs(50);
		HAL_GPIO_WritePin(GPIO_2, GPIO_PIN_7, GPIO_PIN_LOW);
		HAL_DelayMs(50);
	}
}
