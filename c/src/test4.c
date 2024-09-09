#include "mik32_hal_pcc.h"
#include "mik32_hal_gpio.h"
#include "mik32_hal_usart.h"

#include "csr.h"
#include "xprintf.h"

#define TINFO_INFO(c)              ((c) & 0xff)
#define TINFO_ICOUNT(c)            ((c) & (0x1 << 3))
#define TINFO_MCONTROL(c)          ((c) & (0x1 << 2))

#define TDATA1_MCONTROL_LOAD       (0x1 << 0)
#define TDATA1_MCONTROL_STORE      (0x1 << 1)
#define TDATA1_MCONTROL_EXEC       (0x1 << 2)
#define TDATA1_MCONTROL_MODE       (0x1 << 6)
#define TDATA1_MCONTROL_CHAIN      (0x1 << 11)

#define TDATA1_ICOUNT_MODE         (0x1 << 9)

#define TDATA1_ICOUNT_COUNT_S      (10)
#define TDATA1_ICOUNT_COUNT_M      (0x3fff)
#define TDATA1_ICOUNT_COUNT_W(c)   (((c) & TDATA1_ICOUNT_COUNT_M) << TDATA1_ICOUNT_COUNT_S)
#define TDATA1_ICOUNT_COUNT_R(c)   (((c) >> TDATA1_ICOUNT_COUNT_S) & TDATA1_ICOUNT_COUNT_M)

USART_HandleTypeDef husart0;
int ret;

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

void trap_handler(void) __attribute__((interrupt("machine"))) __attribute__((section(".trap")));
void trap_handler(void)
{
	unsigned long mcause;
	unsigned long t1 = 0;
	int step_over = 0;

	__asm__ volatile ("csrr %0, mcause\n\t"
		: "=r" (mcause) : : );

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
	USART_Init();
	GPIO_Init();

	xprintf("%s: enumerate triggers...\n", __func__);

	/* DBTR enumerate procedure */

	for (c = 0; ; c++) {
		write_csr(tselect, c);
		if (ret) {
			xprintf("%s: no %s triggers implemented...\n",
				__func__, c ? "more" : "");
			break;
		}

		v = read_csr(tselect);
		if (v != c) {
			xprintf("%s: no %s triggers implemented...\n",
				__func__, c ? "more" : "");
			break;
		}

		v = read_csr(tinfo);
		if (ret) {
			v = read_csr(tdata1);
			if (ret || (v == 0)) {
				xprintf("%s: no %s triggers implemented...\n",
					__func__, c ? "more" : "");
				break;
			}
		} else {
			if (TINFO_INFO(v) == 0x1) {
				xprintf("%s: no %s triggers implemented...\n",
					__func__, c ? "more" : "");
				break;
			}
		}

		xprintf("%s: trigger %u supported types: [ %s%s]\n", __func__, c,
			TINFO_MCONTROL(v) ? "MCONTROL " : "",
			TINFO_ICOUNT(v) ? "ICOUNT " : "");

		/* check mcontrol trigger properties assuming it is WARL */
		if (TINFO_MCONTROL(v)) {
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

			v = TDATA1_MCONTROL_LOAD  | TDATA1_MCONTROL_STORE |  TDATA1_MCONTROL_EXEC  | TDATA1_MCONTROL_MODE  | TDATA1_MCONTROL_CHAIN;
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
				(v & TDATA1_MCONTROL_MODE)  ? " MODE"  : "",
				(v & TDATA1_MCONTROL_CHAIN) ? " CHAIN" : "");
		}

		/* check icount trigger properties assuming it is WARL */
		if (TINFO_ICOUNT(v)) {
			v = TDATA1_ICOUNT_MODE | TDATA1_ICOUNT_COUNT_W(10);
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
				(v & TDATA1_ICOUNT_MODE)  ? " MODE"  : "",
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
