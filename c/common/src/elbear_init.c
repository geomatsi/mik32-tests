#include "elbear_init.h"

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

void GPIO_LED_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	GPIO_InitStruct.Pin = GPIO_PIN_7;
	GPIO_InitStruct.Mode = HAL_GPIO_MODE_GPIO_OUTPUT;
	GPIO_InitStruct.Pull = HAL_GPIO_PULL_NONE;

	__HAL_PCC_GPIO_2_CLK_ENABLE();
	HAL_GPIO_Init(GPIO_2, &GPIO_InitStruct);
}

void USART_Init(USART_HandleTypeDef *husart)
{
	husart->Instance = UART_0;
	husart->transmitting = Enable;
	husart->receiving = Enable;
	husart->frame = Frame_8bit;
	husart->parity_bit = Disable;
	husart->parity_bit_inversion = Disable;
	husart->bit_direction = LSB_First;
	husart->data_inversion = Disable;
	husart->tx_inversion = Disable;
	husart->rx_inversion = Disable;
	husart->swap = Disable;
	husart->lbm = Disable;
	husart->stop_bit = StopBit_1;
	husart->mode = Asynchronous_Mode;
	husart->xck_mode = XCK_Mode3;
	husart->last_byte_clock = Disable;
	husart->overwrite = Disable;
	husart->rts_mode = AlwaysEnable_mode;
	husart->dma_tx_request = Disable;
	husart->dma_rx_request = Disable;
	husart->channel_mode = Duplex_Mode;
	husart->tx_break_mode = Disable;
	husart->Interrupt.ctsie = Disable;
	husart->Interrupt.eie = Disable;
	husart->Interrupt.idleie = Disable;
	husart->Interrupt.lbdie = Disable;
	husart->Interrupt.peie = Disable;
	husart->Interrupt.rxneie = Disable;
	husart->Interrupt.tcie = Disable;
	husart->Interrupt.txeie = Disable;
	husart->Modem.rts = Disable; //out
	husart->Modem.cts = Disable; //in
	husart->Modem.dtr = Disable; //out
	husart->Modem.dcd = Disable; //in
	husart->Modem.dsr = Disable; //in
	husart->Modem.ri = Disable;  //in
	husart->Modem.ddis = Disable;//out
	husart->baudrate = 115200;

	HAL_USART_Init(husart);
}
