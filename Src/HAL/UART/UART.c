/*
 * UART.c
 *
 *  Created on: Oct 8, 2023
 *      Author: Ali Emad
 */


/*	LIB	*/
#include <stdint.h>

/*	FreeRTOS	*/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/*	MCAL (Ported)	*/
#include "MCAL_Port/Port_UART.h"

/*	SELF	*/
#include "HAL/UART/UART_Config.h"
#include "HAL/UART/UART.h"


/*******************************************************************************
 * Extern HW unit functions:
 ******************************************************************************/
extern void vHOS_UART_HW_init(void);

extern void vHOS_UART_HW_send(uint8_t ucUnitNumber, int8_t* pcArr, uint32_t uiSize);

extern uint8_t ucHOS_UART_HW_receive(		uint8_t ucUnitNumber,
								int8_t* pcInArr,
								uint32_t uiSize,
								TickType_t xTimeout	);

extern uint8_t ucHOS_UART_HW_takeMutex(uint8_t ucUnitNumber, TickType_t xTimeout);

extern void vHOS_UART_HW_releaseMutex(uint8_t ucUnitNumber);

extern uint8_t ucHOS_UART_HW_blockUntilTransmissionComplete(uint8_t ucUnitNumber, TickType_t xTimeout);


/*******************************************************************************
 * Extern SW unit functions:
 ******************************************************************************/
extern void vHOS_UART_SW_init(void);

extern void vHOS_UART_SW_send(uint8_t ucUnitNumber, int8_t* pcArr, uint32_t uiSize);

extern uint8_t ucHOS_UART_SW_receive(		uint8_t ucUnitNumber,
								int8_t* pcInArr,
								uint32_t uiSize,
								TickType_t xTimeout	);

extern uint8_t ucHOS_UART_SW_takeMutex(uint8_t ucUnitNumber, TickType_t xTimeout);

extern void vHOS_UART_SW_releaseMutex(uint8_t ucUnitNumber);

extern uint8_t ucHOS_UART_SW_blockUntilTransmissionComplete(uint8_t ucUnitNumber, TickType_t xTimeout);


/*******************************************************************************
 * Helping macros:
 ******************************************************************************/
/*
 * Defines whether SW UART is used or not.
 */
#define ucIS_SW_UART_USED	(uiCONF_UART_NUMBER_OF_NEEDED_UNITS > portUART_NUMBER_OF_UNITS)

/*
 * Converts an absolute unit number to a relative one. Used when calling SW UART functions.
 */
#define ucCONVERT_TO_SW_UNIT_NUMBER(ucUnitNumber) (ucUnitNumber - portUART_NUMBER_OF_UNITS)

/*******************************************************************************
 * API functions:
 ******************************************************************************/
void vHOS_UART_init(void)
{
	vHOS_UART_HW_init();

#if ucIS_SW_UART_USED
	vHOS_UART_SW_init();
#endif
}

void vHOS_UART_send(uint8_t ucUnitNumber, int8_t* pcArr, uint32_t uiSize)
{
#if ucIS_SW_UART_USED
	if (ucUnitNumber < portUART_NUMBER_OF_UNITS)
		vHOS_UART_HW_send(ucUnitNumber, pcArr, uiSize);
	else
		vHOS_UART_SW_send(ucCONVERT_TO_SW_UNIT_NUMBER(ucUnitNumber), pcArr, uiSize);
#else
	vHOS_UART_HW_send(ucUnitNumber, pcArr, uiSize);
#endif
}

uint8_t ucHOS_UART_receive(		uint8_t ucUnitNumber,
								int8_t* pcInArr,
								uint32_t uiSize,
								TickType_t xTimeout	)
{
#if ucIS_SW_UART_USED
	if (ucUnitNumber < portUART_NUMBER_OF_UNITS)
		return ucHOS_UART_HW_receive(ucUnitNumber, pcInArr, uiSize, xTimeout);
	else
		return ucHOS_UART_SW_receive(ucCONVERT_TO_SW_UNIT_NUMBER(ucUnitNumber), pcInArr, uiSize, xTimeout);
#else
	return ucHOS_UART_HW_receive(ucUnitNumber, pcInArr, uiSize, xTimeout);
#endif
}

uint8_t ucHOS_UART_takeMutex(uint8_t ucUnitNumber, TickType_t xTimeout)
{
#if ucIS_SW_UART_USED
	if (ucUnitNumber < portUART_NUMBER_OF_UNITS)
		return ucHOS_UART_HW_takeMutex(ucUnitNumber, xTimeout);
	else
		return ucHOS_UART_SW_takeMutex(ucCONVERT_TO_SW_UNIT_NUMBER(ucUnitNumber), xTimeout);
#else
	return ucHOS_UART_HW_takeMutex(ucUnitNumber, xTimeout);
#endif
}

void vHOS_UART_releaseMutex(uint8_t ucUnitNumber)
{
#if ucIS_SW_UART_USED
	if (ucUnitNumber < portUART_NUMBER_OF_UNITS)
		vHOS_UART_HW_releaseMutex(ucUnitNumber);
	else
		vHOS_UART_SW_releaseMutex(ucCONVERT_TO_SW_UNIT_NUMBER(ucUnitNumber));
#else
	vHOS_UART_HW_releaseMutex(ucUnitNumber);
#endif
}

uint8_t ucHOS_UART_blockUntilTransmissionComplete(uint8_t ucUnitNumber, TickType_t xTimeout)
{
#if ucIS_SW_UART_USED
	if (ucUnitNumber < portUART_NUMBER_OF_UNITS)
		return ucHOS_UART_HW_blockUntilTransmissionComplete(ucUnitNumber, xTimeout);
	else
		return ucHOS_UART_SW_blockUntilTransmissionComplete(ucCONVERT_TO_SW_UNIT_NUMBER(ucUnitNumber), xTimeout);
#else
	return ucHOS_UART_HW_blockUntilTransmissionComplete(ucUnitNumber, xTimeout);
#endif
}























