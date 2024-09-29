#include "mik32_hal_pcc.h"
#include "mik32_hal_usart.h"

#include "riscv_csr_encoding.h"
#include "xprintf.h"
#include "dbtr.h"
#include "csr.h"

#define SCR1_ICOUNT_DBTR  (2)
#define ILL_INSN (0x00000000)
/* smaller value to fire in trap handler after re-arming */
#define ICOUNT_STEPS (15)
/* larger value to fire in main after re-arming */
//#define ICOUNT_STEPS (20)

USART_HandleTypeDef husart0;

volatile unsigned long count = 0;
volatile unsigned long traps = 0;

void SystemClock_Config(void)
{
	PCC_InitTypeDef PCC_OscInit = {0};

	PCC_OscInit.OscillatorEnable = PCC_OSCILLATORTYPE_ALL;
	PCC_OscInit.FreqMon.OscillatorSystem = PCC_OSCILLATORTYPE_OSC32M;
	PCC_OscInit.FreqMon.ForceOscSys = PCC_FORCE_OSC_SYS_UNFIXED;
	PCC_OscInit.FreqMon.Force32KClk = PCC_FREQ_MONITOR_SOURCE_OSC32K;
	PCC_OscInit.AHBDivider = 0;
	PCC_OscInit.APBMDivider = 0;
	PCC_OscInit.APBPDivider = 0;
	PCC_OscInit.HSI32MCalibrationValue = 128;
	PCC_OscInit.LSI32KCalibrationValue = 128;
	PCC_OscInit.RTCClockSelection = PCC_RTC_CLOCK_SOURCE_AUTO;
	PCC_OscInit.RTCClockCPUSelection = PCC_CPU_RTC_CLOCK_SOURCE_OSC32K;

	HAL_PCC_Config(&PCC_OscInit);
}

void USART_Init()
{
	husart0.Instance = UART_0;
	husart0.transmitting = Enable;
	husart0.receiving = Enable;
	husart0.frame = Frame_8bit;
	husart0.parity_bit = Disable;
	husart0.parity_bit_inversion = Disable;
	husart0.bit_direction = LSB_First;
	husart0.data_inversion = Disable;
	husart0.tx_inversion = Disable;
	husart0.rx_inversion = Disable;
	husart0.swap = Disable;
	husart0.lbm = Disable;
	husart0.stop_bit = StopBit_1;
	husart0.mode = Asynchronous_Mode;
	husart0.xck_mode = XCK_Mode3;
	husart0.last_byte_clock = Disable;
	husart0.overwrite = Disable;
	husart0.rts_mode = AlwaysEnable_mode;
	husart0.dma_tx_request = Disable;
	husart0.dma_rx_request = Disable;
	husart0.channel_mode = Duplex_Mode;
	husart0.tx_break_mode = Disable;
	husart0.Interrupt.ctsie = Disable;
	husart0.Interrupt.eie = Disable;
	husart0.Interrupt.idleie = Disable;
	husart0.Interrupt.lbdie = Disable;
	husart0.Interrupt.peie = Disable;
	husart0.Interrupt.rxneie = Disable;
	husart0.Interrupt.tcie = Disable;
	husart0.Interrupt.txeie = Disable;
	husart0.Modem.rts = Disable; //out
	husart0.Modem.cts = Disable; //in
	husart0.Modem.dtr = Disable; //out
	husart0.Modem.dcd = Disable; //in
	husart0.Modem.dsr = Disable; //in
	husart0.Modem.ri = Disable;  //in
	husart0.Modem.ddis = Disable;//out
	husart0.baudrate = 115200;

	HAL_USART_Init(&husart0);
}

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
	USART_Init();

#if 1
	enable_icount_trigger(ICOUNT_STEPS);
#else
	__asm__ volatile (".word %0" : : "i"(ILL_INSN) : );
#endif

	while (1) {
		count++;
	}
}
