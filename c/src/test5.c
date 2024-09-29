#include "elbear_init.h"
#include "dbtr.h"

#include "riscv_csr_encoding.h"
#include "xprintf.h"
#include "csr.h"

USART_HandleTypeDef husart0;
unsigned long rdata = 10;
unsigned long wdata = 10;
int eret;

int enable_trigger(unsigned long c)
{
	unsigned long v;
	void *addr;

	write_csr(tselect, c);
	if (eret == CAUSE_ILLEGAL_INSTRUCTION) {
		xprintf("%s(%d): trigger #%lu not available...\n",
				__func__, __LINE__, c);
		return -1;
	}

	v = read_csr(tselect);
	if (v != c) {
		xprintf("%s(%d): trigger #%lu not available...\n",
				__func__, __LINE__, c);
		return -1;
	}

	write_csr(tdata1, 0x0);
	if (eret == CAUSE_ILLEGAL_INSTRUCTION) {
		xprintf("%s: failed to zero tdata1 for trigger #%lu...\n",
				__func__, c);
		return -1;
	}

	switch (c) {
		case 0: /* trigger #0: read watchpoint */
			v = TDATA1_MCONTROL_LOAD  | TDATA1_MCONTROL_MODE_M |
				TDATA1_MCONTROL_MATCH_W(TDATA1_MCONTROL_MATCH_EQUAL) |
				TDATA1_MCONTROL_ACTION_W(TDATA1_MCONTROL_ACTION_EBREAK);
			addr = &rdata;
			break;
		case 1: /* trigger #1: write watchpoint */
			v = TDATA1_MCONTROL_STORE | TDATA1_MCONTROL_MODE_M |
				TDATA1_MCONTROL_MATCH_W(TDATA1_MCONTROL_MATCH_EQUAL) |
				TDATA1_MCONTROL_ACTION_W(TDATA1_MCONTROL_ACTION_EBREAK);
			addr = &wdata;
			break;
		default:
			xprintf("%s: unexpected trigger #%lu...\n",
				__func__, c);
			return -1;
	}

	write_csr(tdata2, addr);
	if (eret == CAUSE_ILLEGAL_INSTRUCTION) {
		xprintf("%s: failed to set tdata2 for trigger #%lu...\n",
				__func__, c);
		return -1;
	}

	write_csr(tdata1, v);
	if (eret == CAUSE_ILLEGAL_INSTRUCTION) {
		xprintf("%s: failed to set tdata1 for trigger #%lu...\n",
				__func__, c);
		return -1;
	}

	return 0;
}

int disable_trigger(unsigned long c)
{
	unsigned long v;

	write_csr(tselect, c);
	if (eret == CAUSE_ILLEGAL_INSTRUCTION) {
		xprintf("%s: trigger #%lu not available...\n",
				__func__, c);
		return -1;
	}

	v = read_csr(tselect);
	if (v != c) {
		xprintf("%s(%d): trigger #%lu not available...\n",
				__func__, __LINE__, c);
		return -1;
	}

	write_csr(tdata1, 0x0);
	if (eret == CAUSE_ILLEGAL_INSTRUCTION) {
		xprintf("%s: failed to zero tdata1 for trigger #%lu...\n",
				__func__, c);
		return -1;
	}

	write_csr(tdata2, 0x0);
	if (eret == CAUSE_ILLEGAL_INSTRUCTION) {
		xprintf("%s: failed to zero tdata2 for trigger #%lu...\n",
				__func__, c);
		return -1;
	}

	return 0;
}

int check_trigger(unsigned long c, unsigned long *hit)
{
	unsigned long v;

	write_csr(tselect, c);
	if (eret == CAUSE_ILLEGAL_INSTRUCTION) {
		xprintf("%s: trigger #%lu not available...\n",
				__func__, c);
		return -1;
	}

	v = read_csr(tselect);
	if (v != c) {
		xprintf("%s(%d): trigger #%lu not available...\n",
				__func__, __LINE__, c);
		return -1;
	}

	v = read_csr(tdata1);
	if (eret == CAUSE_ILLEGAL_INSTRUCTION) {
		xprintf("%s: failed to read tdata1 from trigger #%lu...\n",
				__func__, c);
		return -1;
	}

	/* note: hit bit may not be implemented in h/w */
	*hit = !!(v & TDATA1_MCONTROL_HIT);

	return 0;
}

int dump_trigger(unsigned long c)
{
	unsigned long r, v;

	write_csr(tselect, c);
	if (eret == CAUSE_ILLEGAL_INSTRUCTION) {
		xprintf("%s: trigger #%lu not available...\n",
				__func__, c);
		return -1;
	}

	v = read_csr(tselect);
	if (v != c) {
		xprintf("%s(%d): trigger #%lu not available...\n",
				__func__, __LINE__, c);
		return -1;
	}

	v = read_csr(tdata1);
	if (eret == CAUSE_ILLEGAL_INSTRUCTION) {
		xprintf("%s: failed to read tdata1 from trigger #%lu...\n",
				__func__, c);
		return -1;
	}

	r = read_csr(mtval);

	xprintf("trigger #%lu: mtval[0x%lx] hit[%d]\n",
		c, r, !!(v & TDATA1_MCONTROL_HIT));

	return 0;
}

void trap_handler(void) __attribute__((interrupt("machine"))) __attribute__((section(".trap")));
void trap_handler(void)
{
	unsigned long hit, trig, mcause;
	unsigned long t1 = 0;
	int step_over = 0;
	int ret;

	mcause = read_csr(mcause);

	/* reset trap status */
	eret = 0;

	switch (mcause) {
		case CAUSE_ILLEGAL_INSTRUCTION:
			eret = CAUSE_ILLEGAL_INSTRUCTION;
			step_over = 1;
			break;
		case CAUSE_BREAKPOINT:
			eret = CAUSE_BREAKPOINT;
			for (trig = 0; trig < 2; trig++) {
				dump_trigger(trig);
				ret = check_trigger(trig, &hit);
				if (ret < 0)
					goto error;
				if (hit) {
					xprintf("%s: disable fired trigger #%lu...\n",
						__func__, trig);
					ret = disable_trigger(trig);
					if (ret < 0)
						goto error;
				}
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
	unsigned long cnt = 0;
	int ret;

	SystemClock_Config();
	USART_Init(&husart0);
	GPIO_LED_Init();

	xprintf("%s: configure triggers as watchpoints...\n", __func__);

	while (1) {
		volatile unsigned long t1, t2;
		unsigned long trig;

		t1 = rdata++;
		wdata = t1;
		t2 = wdata;

		xprintf("%s[%lu]: rdata[%lu] wdata[%lu]...\n",
				__func__, cnt++, t1, t2);
		HAL_GPIO_TogglePin(GPIO_2, GPIO_PIN_7);
		HAL_DelayMs(1000);

		/* each cycle enable different trigger */
		trig = cnt % 2;
		xprintf("%s: enable trigger #%lu...\n", __func__, trig);
		ret = enable_trigger(trig);
		if (ret < 0) {
			xprintf("%s: failed to enable trigger #%lu...\n",
					__func__, trig);
			goto failure;
		}
	}

failure:
	while (1) {
		HAL_GPIO_TogglePin(GPIO_2, GPIO_PIN_7);
		HAL_DelayMs(30);
	}

	return;
}
