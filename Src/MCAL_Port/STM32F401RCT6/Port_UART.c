/*
 * Port_UART.c
 *
 *  Created on: Sep 12, 2023
 *      Author: Ali Emad
 */

/*	Target checking	*/
#include "MCAL_Port/Port_Target.h"
#ifdef MCAL_PORT_TARGET_STM32F401RCT6


#include "MCAL_Port/Port_Clock.h"
#include "MCAL_Port/Port_Interrupt.h"

/*******************************************************************************
 * Units.
 ******************************************************************************/
USART_TypeDef* const pxPortUartArr[] = {USART1, USART2, USART6};

/*******************************************************************************
 * Units' clock sources.
 ******************************************************************************/
uint32_t puiPortUartPeriphClockArr[] = {uiPORT_CLOCK_MAIN_HZ, uiPORT_CLOCK_MAIN_HZ/2, uiPORT_CLOCK_MAIN_HZ};

/*******************************************************************************
 * Callbacks.
 ******************************************************************************/
#include "MCAL_Port/Port_UART.h"


#ifdef ucPORT_INTERRUPT_IRQ_DEF_UART

void (*ppfPortUartTxeCallbackArr[portUART_NUMBER_OF_UNITS])(void*);
void* ppvPortUartTxeCallbackParamsArr[portUART_NUMBER_OF_UNITS];

void (*ppfPortUartRxneCallbackArr[portUART_NUMBER_OF_UNITS])(void*);
void* ppvPortUartRxneCallbackParamsArr[portUART_NUMBER_OF_UNITS];

void (*ppfPortUartTcCallbackArr[portUART_NUMBER_OF_UNITS])(void*);
void* ppvPortUartTcCallbackParamsArr[portUART_NUMBER_OF_UNITS];

void USART1_IRQHandler(void)
{
#define ucUNIT_NUMBER		0
	if (ucPORT_UART_GET_TC_FLAG(ucUNIT_NUMBER) && ucPORT_UART_IS_TC_INTERRUPT_ENABLED(ucUNIT_NUMBER))
	{
		ppfPortUartTcCallbackArr[ucUNIT_NUMBER](ppvPortUartTcCallbackParamsArr[ucUNIT_NUMBER]);
		vPORT_UART_CLEAR_TC_FLAG(ucUNIT_NUMBER);
	}

	else if (ucPORT_UART_GET_RXNE_FLAG(ucUNIT_NUMBER) && ucPORT_UART_IS_RXNE_INTERRUPT_ENABLED(ucUNIT_NUMBER))
	{
		ppfPortUartRxneCallbackArr[ucUNIT_NUMBER](ppvPortUartRxneCallbackParamsArr[ucUNIT_NUMBER]);
		vPORT_UART_CLEAR_RXNE_FLAG(ucUNIT_NUMBER);
	}

	else if (ucPORT_UART_GET_TXE_FLAG(ucUNIT_NUMBER) && ucPORT_UART_IS_TXE_INTERRUPT_ENABLED(ucUNIT_NUMBER))
	{
		ppfPortUartTxeCallbackArr[ucUNIT_NUMBER](ppvPortUartTxeCallbackParamsArr[ucUNIT_NUMBER]);
	}

	/*
	 * If overrun error is causing the handler call, clear its flag to avoid having
	 * the handler starving the OS.
	 */
	else if (ucPORT_UART_GET_ORE_FLAG(ucUNIT_NUMBER))
	{
		vPORT_UART_CLEAR_ORE_FLAG(ucUNIT_NUMBER);
	}

#undef ucUNIT_NUMBER
}

void USART2_IRQHandler(void)
{
#define ucUNIT_NUMBER		1

	if (ucPORT_UART_GET_TC_FLAG(ucUNIT_NUMBER) && ucPORT_UART_IS_TC_INTERRUPT_ENABLED(ucUNIT_NUMBER))
	{
		ppfPortUartTcCallbackArr[ucUNIT_NUMBER](ppvPortUartTcCallbackParamsArr[ucUNIT_NUMBER]);
	}

	else if (ucPORT_UART_GET_RXNE_FLAG(ucUNIT_NUMBER) && ucPORT_UART_IS_RXNE_INTERRUPT_ENABLED(ucUNIT_NUMBER))
	{
		ppfPortUartRxneCallbackArr[ucUNIT_NUMBER](ppvPortUartRxneCallbackParamsArr[ucUNIT_NUMBER]);
		vPORT_UART_CLEAR_RXNE_FLAG(ucUNIT_NUMBER);
	}

	else if (ucPORT_UART_GET_TXE_FLAG(ucUNIT_NUMBER) && ucPORT_UART_IS_TXE_INTERRUPT_ENABLED(ucUNIT_NUMBER))
	{
		ppfPortUartTxeCallbackArr[ucUNIT_NUMBER](ppvPortUartTxeCallbackParamsArr[ucUNIT_NUMBER]);
	}

	/*
	 * If overrun error is causing the handler call, clear its flag to avoid having
	 * the handler starving the OS.
	 */
	else if (ucPORT_UART_GET_ORE_FLAG(ucUNIT_NUMBER))
	{
		vPORT_UART_CLEAR_ORE_FLAG(ucUNIT_NUMBER);
	}

#undef ucUNIT_NUMBER
}

void USART3_IRQHandler(void)
{
#define ucUNIT_NUMBER		2

	if (ucPORT_UART_GET_TC_FLAG(ucUNIT_NUMBER) && ucPORT_UART_IS_TC_INTERRUPT_ENABLED(ucUNIT_NUMBER))
	{
		ppfPortUartTcCallbackArr[ucUNIT_NUMBER](ppvPortUartTcCallbackParamsArr[ucUNIT_NUMBER]);
	}

	else if (ucPORT_UART_GET_RXNE_FLAG(ucUNIT_NUMBER) && ucPORT_UART_IS_RXNE_INTERRUPT_ENABLED(ucUNIT_NUMBER))
	{
		ppfPortUartRxneCallbackArr[ucUNIT_NUMBER](ppvPortUartRxneCallbackParamsArr[ucUNIT_NUMBER]);
		vPORT_UART_CLEAR_RXNE_FLAG(ucUNIT_NUMBER);
	}

	else if (ucPORT_UART_GET_TXE_FLAG(ucUNIT_NUMBER) && ucPORT_UART_IS_TXE_INTERRUPT_ENABLED(ucUNIT_NUMBER))
	{
		ppfPortUartTxeCallbackArr[ucUNIT_NUMBER](ppvPortUartTxeCallbackParamsArr[ucUNIT_NUMBER]);
	}

	/*
	 * If overrun error is causing the handler call, clear its flag to avoid having
	 * the handler starving the OS.
	 */
	else if (ucPORT_UART_GET_ORE_FLAG(ucUNIT_NUMBER))
	{
		vPORT_UART_CLEAR_ORE_FLAG(ucUNIT_NUMBER);
	}

#undef ucUNIT_NUMBER
}

#endif


#endif /* Target checking */
