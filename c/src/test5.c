#include "mik32_hal_pcc.h"
#include "mik32_hal_gpio.h"
#include "mik32_hal_usart.h"

#include "riscv_csr_encoding.h"
#include "xprintf.h"
#include "dbtr.h"
#include "csr.h"

USART_HandleTypeDef husart0;
unsigned long rdata = 10;
unsigned long wdata = 10;
int eret;

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

void GPIO_Init()
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	GPIO_InitStruct.Pin = GPIO_PIN_7;
	GPIO_InitStruct.Mode = HAL_GPIO_MODE_GPIO_OUTPUT;
	GPIO_InitStruct.Pull = HAL_GPIO_PULL_NONE;

	__HAL_PCC_GPIO_2_CLK_ENABLE();
	HAL_GPIO_Init(GPIO_2, &GPIO_InitStruct);
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
	USART_Init();
	GPIO_Init();

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
