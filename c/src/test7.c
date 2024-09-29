#include "riscv_csr_encoding.h"
#include "xprintf.h"
#include "csr.h"

#include "elbear_init.h"
#include "dbtr.h"

#define SCR1_ICOUNT_DBTR  (2)
#define ICOUNT_STEPS      (100)

USART_HandleTypeDef husart0;

volatile unsigned long count = 0;
volatile int eret;

int enable_icount_trigger(unsigned int steps)
{
	unsigned long trig = SCR1_ICOUNT_DBTR;
	unsigned long v;

	write_csr(tselect, trig);
	if (eret == CAUSE_ILLEGAL_INSTRUCTION) {
		xprintf("%s(%d): trigger #%lu not available...\n",
				__func__, __LINE__, trig);
		return -1;
	}

	v = read_csr(tselect);
	if (v != trig) {
		xprintf("%s(%d): trigger #%lu not available...\n",
				__func__, __LINE__, trig);
		return -1;
	}

	write_csr(tdata1, 0x0);
	if (eret == CAUSE_ILLEGAL_INSTRUCTION) {
		xprintf("%s: failed to zero tdata1 for trigger #%lu...\n",
				__func__, trig);
		return -1;
	}

	v = TDATA1_ICOUNT_MODE_M | TDATA1_ICOUNT_COUNT_W(steps) |
		TDATA1_ICOUNT_ACTION_W(TDATA1_MCONTROL_ACTION_EBREAK);

	write_csr(tdata1, v);
	if (eret == CAUSE_ILLEGAL_INSTRUCTION) {
		xprintf("%s: failed to set tdata1 for trigger #%lu...\n",
				__func__, trig);
		return -1;
	}

	return 0;
}

int disable_icount_trigger(void)
{
	unsigned long trig = SCR1_ICOUNT_DBTR;
	unsigned long v;

	write_csr(tselect, trig);
	if (eret == CAUSE_ILLEGAL_INSTRUCTION) {
		xprintf("%s: trigger #%lu not available...\n",
				__func__, trig);
		return -1;
	}

	v = read_csr(tselect);
	if (v != trig) {
		xprintf("%s(%d): trigger #%lu not available...\n",
				__func__, __LINE__, trig);
		return -1;
	}

	write_csr(tdata1, 0x0);
	if (eret == CAUSE_ILLEGAL_INSTRUCTION) {
		xprintf("%s: failed to zero tdata1 for trigger #%lu...\n",
				__func__, trig);
		return -1;
	}

	return 0;
}

int check_icount_trigger(unsigned long *hit)
{
	unsigned long trig = SCR1_ICOUNT_DBTR;
	unsigned long v;

	write_csr(tselect, trig);
	if (eret == CAUSE_ILLEGAL_INSTRUCTION) {
		xprintf("%s: trigger #%lu not available...\n",
				__func__, trig);
		return -1;
	}

	v = read_csr(tselect);
	if (v != trig) {
		xprintf("%s(%d): trigger #%lu not available...\n",
				__func__, __LINE__, trig);
		return -1;
	}

	v = read_csr(tdata1);
	if (eret == CAUSE_ILLEGAL_INSTRUCTION) {
		xprintf("%s: failed to read tdata1 from trigger #%lu...\n",
				__func__, trig);
		return -1;
	}

	/* note: hit bit may not be implemented in h/w */
	*hit = !!(v & TDATA1_ICOUNT_HIT);

	return 0;
}

int dump_icount_trigger(void)
{
	unsigned long trig = SCR1_ICOUNT_DBTR;
	unsigned long r, v;

	write_csr(tselect, trig);
	if (eret == CAUSE_ILLEGAL_INSTRUCTION) {
		xprintf("%s: trigger #%lu not available...\n",
				__func__, trig);
		return -1;
	}

	v = read_csr(tselect);
	if (v != trig) {
		xprintf("%s(%d): trigger #%lu not available...\n",
				__func__, __LINE__, trig);
		return -1;
	}

	v = read_csr(tdata1);
	if (eret == CAUSE_ILLEGAL_INSTRUCTION) {
		xprintf("%s: failed to read tdata1 from trigger #%lu...\n",
				__func__, trig);
		return -1;
	}

	r = read_csr(mtval);

	xprintf("trigger #%lu: mtval[0x%lx] hit[%d] count[%d]\n",
		trig, r, !!(v & TDATA1_ICOUNT_HIT), TDATA1_ICOUNT_COUNT_R(v));

	return 0;
}

void trap_handler(void) __attribute__((interrupt("machine"))) __attribute__((section(".trap")));
void trap_handler(void)
{
	unsigned long hit, mcause, mepc;
	unsigned long t1 = 0;
	int step_over = 0;
	int ret;

	__asm__ volatile ("csrr %0, mcause\n\t"
		: "=r" (mcause) : : );

	__asm__ volatile ("csrr %0, mepc\n\t"
		: "=r" (mepc) : : );

	/* reset trap status */
	eret = 0;

	switch (mcause) {
		case CAUSE_ILLEGAL_INSTRUCTION:
			eret = CAUSE_ILLEGAL_INSTRUCTION;
			step_over = 1;
			break;
		case CAUSE_BREAKPOINT:
			eret = CAUSE_BREAKPOINT;
			ret = check_icount_trigger(&hit);
			if (ret < 0)
				goto error;
			if (hit) {
				ret = dump_icount_trigger();
				if (ret < 0)
					goto error;

				ret = disable_icount_trigger();
				if (ret < 0)
					goto error;

				xprintf("%s: mepc(0x%lx) count(%lu)\n",
					__func__, mepc, count);
				xprintf("%s: wait 500ms and re-enable icount trigger...\n",
					__func__);
				HAL_DelayMs(500);

				/* note: beware small values of icount to avoid firing before mret */
				ret = enable_icount_trigger(ICOUNT_STEPS);
				if (ret < 0)
					goto error;
			}
			step_over = 0;
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
	HAL_GPIO_WritePin(GPIO_2, GPIO_PIN_7, GPIO_PIN_HIGH);
	xprintf("%s: error condition in exception...\n", __func__);

	while (1) {
		__asm__ volatile ("wfi");
	}

	return;
}

void main(void)
{
	int ret;

	SystemClock_Config();
	USART_Init(&husart0);
	GPIO_LED_Init();

	ret = enable_icount_trigger(ICOUNT_STEPS);
	if (ret < 0)
		goto failure;

	while (1) {
		count++;
	}

failure:
	while (1) {
		HAL_GPIO_TogglePin(GPIO_2, GPIO_PIN_7);
		HAL_DelayMs(30);
	}

	return;
}
