/*
 * Port_UART.h
 *
 *  Created on: Sep 12, 2023
 *      Author: Ali Emad
 */

/*	Target checking	*/
#include "MCAL_Port/Port_Target.h"
#ifdef MCAL_PORT_TARGET_STM32F103C8T6


#ifndef COTS_OS_INC_MCAL_PORT_PORT_UART_H_
#define COTS_OS_INC_MCAL_PORT_PORT_UART_H_

#include "stm32f1xx.h"
#include "stm32f1xx_ll_usart.h"

extern USART_TypeDef* const pxPortUartArr[];
extern uint32_t puiPortUartPeriphClockArr[];

extern void (*ppfPortUartTxeCallbackArr[])(void*);
extern void* ppvPortUartTxeCallbackParamsArr[];

extern void (*ppfPortUartRxneCallbackArr[])(void*);
extern void* ppvPortUartRxneCallbackParamsArr[];

extern void (*ppfPortUartTcCallbackArr[])(void*);
extern void* ppvPortUartTcCallbackParamsArr[];

/*	Number of units available in the used target	*/
#define portUART_NUMBER_OF_UNITS		3

/*	Initializes UART HW	*/
static inline void vPort_UART_initHW(uint8_t ucUnitNumber)
{
	LL_USART_SetDataWidth(pxPortUartArr[ucUnitNumber], LL_USART_DATAWIDTH_8B);
}

/*	Enables UART unit.	*/
static inline void vPort_UART_enable(uint8_t ucUnitNumber)
{
	LL_USART_Enable(pxPortUartArr[ucUnitNumber]);
}

/*	Disables UART unit.	*/
static inline void vPort_UART_disable(uint8_t ucUnitNumber)
{
	LL_USART_Disable(pxPortUartArr[ucUnitNumber]);
}

/*
 * Sets transfer direction.
 *
 * Note:
 *	"ucDir" :
 *		-	0 ==> Enable Rx only.
 *		-	1 ==> Enable Tx only.
 *		-	2 ==> Enable both Rx & Tx.
 */
static inline void vPort_UART_setTransferDirection(uint8_t ucUnitNumber, uint8_t ucDir)
{
	uint32_t uiDirVal = 0;
	switch (ucDir)
	{
	case 0:	uiDirVal = LL_USART_DIRECTION_RX;		break;
	case 1:	uiDirVal = LL_USART_DIRECTION_RX;		break;
	case 2:	uiDirVal = LL_USART_DIRECTION_TX_RX;	break;
	}

	LL_USART_SetTransferDirection(pxPortUartArr[ucUnitNumber], uiDirVal);
}

/*
 * Sets stop bits length.
 *
 * Note:
 * 	"ucLen":
 * 		-	0 ==> Use 0.5-bit stop bit.
 * 		-	1 ==> Use 1-bit stop bit.
 * 		-	2 ==> Use 1.5-bit stop bit.
 * 		-	3 ==> Use 2-bit stop bit.
 */
static inline void vPort_UART_setStopBitsLength(uint8_t ucUnitNumber, uint8_t ucLen)
{
	uint32_t uiLenVal = 0;
	switch (ucLen)
	{
	case 0:	uiLenVal = LL_USART_STOPBITS_0_5;		break;
	case 1:	uiLenVal = LL_USART_STOPBITS_1;			break;
	case 2:	uiLenVal = LL_USART_STOPBITS_1_5;		break;
	case 3:	uiLenVal = LL_USART_STOPBITS_2;			break;
	}

	LL_USART_SetStopBitsLength(pxPortUartArr[ucUnitNumber], uiLenVal);
}

/*
 * Sets baud-rate.
 */
static inline void vPort_UART_setBaudRate(uint8_t ucUnitNumber, uint32_t uiBaudRate)
{
	LL_USART_SetBaudRate(	pxPortUartArr[ucUnitNumber],
							puiPortUartPeriphClockArr[ucUnitNumber],
							uiBaudRate	);
}

/*
 * Transmits 1 byte.
 */
static inline void vPort_UART_sendByte(uint8_t ucUnitNumber, uint8_t ucByte)
{
	LL_USART_TransmitData8(pxPortUartArr[ucUnitNumber], ucByte);
}

/*
 * Reads 1 byte.
 */
static inline uint8_t ucPort_UART_readByte(uint8_t ucUnitNumber)
{
	return LL_USART_ReceiveData8(pxPortUartArr[ucUnitNumber]);
}

/*	Enables TxE interrupt	*/
static inline void vPort_UART_enableTxeInterrupt(uint8_t ucUnitNumber)
{
	LL_USART_EnableIT_TXE(pxPortUartArr[ucUnitNumber]);
}

/*	Disables TxE interrupt	*/
static inline void vPort_UART_disableTxeInterrupt(uint8_t ucUnitNumber)
{
	LL_USART_DisableIT_TXE(pxPortUartArr[ucUnitNumber]);
}

/*	Checks whether TxE interrupt is enabled or not	*/
#define ucPORT_UART_IS_TXE_INTERRUPT_ENABLED(ucUnitNumber)	\
	(LL_USART_IsEnabledIT_TXE(pxPortUartArr[(ucUnitNumber)]))

/*	Enables RxNE interrupt	*/
static inline void vPort_UART_enableRxneInterrupt(uint8_t ucUnitNumber)
{
	LL_USART_EnableIT_RXNE(pxPortUartArr[ucUnitNumber]);
}

/*	Disables RxNE interrupt	*/
static inline void vPort_UART_disableRxneInterrupt(uint8_t ucUnitNumber)
{
	LL_USART_DisableIT_RXNE(pxPortUartArr[ucUnitNumber]);
}

/*	Checks whether RxNE interrupt is enabled or not	*/
#define ucPORT_UART_IS_RXNE_INTERRUPT_ENABLED(ucUnitNumber)	\
	(LL_USART_IsEnabledIT_RXNE(pxPortUartArr[(ucUnitNumber)]))

/*	Enables TC interrupt	*/
static inline void vPort_UART_enableTcInterrupt(uint8_t ucUnitNumber)
{
	LL_USART_EnableIT_TC(pxPortUartArr[ucUnitNumber]);
}

/*	Disables TC interrupt	*/
static inline void vPort_UART_disableTcInterrupt(uint8_t ucUnitNumber)
{
	LL_USART_DisableIT_TC(pxPortUartArr[ucUnitNumber]);
}

/*	Checks whether TC interrupt is enabled or not	*/
#define ucPORT_UART_IS_TC_INTERRUPT_ENABLED(ucUnitNumber)	\
	(LL_USART_IsEnabledIT_TC(pxPortUartArr[(ucUnitNumber)]))

/*	Reads TxE flag	*/
#define ucPORT_UART_GET_TXE_FLAG(ucUnitNumber)	\
	(LL_USART_IsActiveFlag_TXE(pxPortUartArr[(ucUnitNumber)]))

/*	Reads RxNE flag	*/
#define ucPORT_UART_GET_RXNE_FLAG(ucUnitNumber)	\
	(LL_USART_IsActiveFlag_RXNE(pxPortUartArr[(ucUnitNumber)]))

/*	Reads TC flag	*/
#define ucPORT_UART_GET_TC_FLAG(ucUnitNumber)	\
	(LL_USART_IsActiveFlag_TC(pxPortUartArr[(ucUnitNumber)]))

/*	Reads ORE (Overrun error) flag	*/
#define ucPORT_UART_GET_ORE_FLAG(ucUnitNumber)	\
	(LL_USART_IsActiveFlag_ORE(pxPortUartArr[(ucUnitNumber)]))

/*	Clears RxNE flag	*/
#define vPORT_UART_CLEAR_RXNE_FLAG(ucUnitNumber)	\
	(LL_USART_ClearFlag_RXNE(pxPortUartArr[(ucUnitNumber)]))

/*	Clears TC flag	*/
#define vPORT_UART_CLEAR_TC_FLAG(ucUnitNumber)	\
	(LL_USART_ClearFlag_TC(pxPortUartArr[(ucUnitNumber)]))

/*	Clears ORE flag	*/
#define vPORT_UART_CLEAR_ORE_FLAG(ucUnitNumber)	\
	(LL_USART_ClearFlag_ORE(pxPortUartArr[(ucUnitNumber)]))

/*	Sets TxE callback	*/
static inline void vPort_UART_setTxeCallback(	uint8_t ucUnitNumber,
												void(*pfCallback)(void*),
												void* pvParams	)
{
	ppfPortUartTxeCallbackArr[ucUnitNumber] = pfCallback;
	ppvPortUartTxeCallbackParamsArr[ucUnitNumber] = pvParams;
}

/*	Sets RxNE callback	*/
static inline void vPort_UART_setRxneCallback(	uint8_t ucUnitNumber,
												void(*pfCallback)(void*),
												void* pvParams	)
{
	ppfPortUartRxneCallbackArr[ucUnitNumber] = pfCallback;
	ppvPortUartRxneCallbackParamsArr[ucUnitNumber] = pvParams;
}

/*	Sets TC callback	*/
static inline void vPort_UART_setTcCallback(	uint8_t ucUnitNumber,
												void(*pfCallback)(void*),
												void* pvParams	)
{
	ppfPortUartTcCallbackArr[ucUnitNumber] = pfCallback;
	ppvPortUartTcCallbackParamsArr[ucUnitNumber] = pvParams;
}











#endif /* COTS_OS_INC_MCAL_PORT_PORT_UART_H_ */


#endif /* Target checking */