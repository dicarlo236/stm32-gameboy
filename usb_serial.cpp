#include "stm32h7xx.h"
#include "usb_serial.h"

UART_HandleTypeDef UartHandle;

void Init_USB_Serial(uint32_t baud) {
	UartHandle.Instance = USARTx;
	UartHandle.Init.BaudRate   = baud;
	UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
	UartHandle.Init.StopBits   = UART_STOPBITS_1;
	UartHandle.Init.Parity     = UART_PARITY_NONE;
	UartHandle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
	UartHandle.Init.Mode       = UART_MODE_TX_RX;
	UartHandle.Init.OverSampling = UART_OVERSAMPLING_16;

	HAL_UART_Init(&UartHandle);
}
