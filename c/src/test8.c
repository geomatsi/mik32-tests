#include "riscv_csr_encoding.h"
#include "xprintf.h"
#include "csr.h"

#include "elbear_init.h"
#include "dbtr.h"

#define SCR1_ICOUNT_DBTR  (2)
#define ILL_INSN (0x00000000)
/* smaller value to fire in trap handler after re-arming */
#define ICOUNT_STEPS (15)
/* larger value to fire in main after re-arming */
//#define ICOUNT_STEPS (20)

USART_HandleTypeDef husart0;

volatile unsigned long count = 0;
volatile unsigned long traps = 0;

inline void enable_icount_trigger(unsigned int steps)
{
	unsigned long v;

	write_csr(tselect, SCR1_ICOUNT_DBTR);
	v = TDATA1_ICOUNT_MODE_M | TDATA1_ICOUNT_COUNT_W(steps) |
		TDATA1_ICOUNT_ACTION_W(TDATA1_MCONTROL_ACTION_EBREAK);
	write_csr(tdata1, v);
}

inline void disable_icount_trigger(void)
{
	write_csr(tselect, SCR1_ICOUNT_DBTR);
	write_csr(tdata1, 0x0);
}

inline int check_icount_trigger(void)
{
	write_csr(tselect, SCR1_ICOUNT_DBTR);

	return !!(read_csr(tdata1) & TDATA1_ICOUNT_HIT);
}

void trap_handler(void) __attribute__((interrupt("machine"))) __attribute__((section(".trap")));
void trap_handler(void)
{
	unsigned long mcause, mtval, mepc;

	mcause = read_csr(mcause);
	mtval = read_csr(mtval);
	mepc = read_csr(mepc);

	xprintf("%s: mcause(0x%lx) mtval(0x%lx) mepc(0x%lx) count(%lu) traps(%lu)\n",
		__func__, mcause, mtval, mepc, count, traps++);

	switch (mcause) {
		case CAUSE_ILLEGAL_INSTRUCTION:
			__asm__ volatile (".word %0" : : "i"(ILL_INSN) : );
			break;
		case CAUSE_BREAKPOINT:
			if (check_icount_trigger())
				enable_icount_trigger(ICOUNT_STEPS);
			break;
		default:
			xprintf("%s: unexpected exception: 0x%lx...\n",
				__func__, mcause);
			goto error;
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

#if 1
	enable_icount_trigger(ICOUNT_STEPS);
#else
	__asm__ volatile (".word %0" : : "i"(ILL_INSN) : );
#endif

	while (1) {
		count++;
	}
}
