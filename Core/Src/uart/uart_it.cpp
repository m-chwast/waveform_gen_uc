#include "usart.h"
#include "uart/uart.h"




void HAL_UART_TxCpltCallback(UART_HandleTypeDef * huart)
{
	if(huart == &huart2)
	{
		uart.transmit_next();
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef * huart)
{
	if(huart == &huart2)
	{
		uart.receive();
	}
}

