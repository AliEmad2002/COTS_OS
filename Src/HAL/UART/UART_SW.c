/*
 * UART_SW.c
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
#include "MCAL_Port/Port_Interrupt.h"
#include "MCAL_Port/Port_GPIO.h"
#include "MCAL_Port/Port_AFIO.h"
#include "MCAL_Port/Port_DIO.h"
#include "MCAL_Port/Port_EXTI.h"

/*	HAL (OS)	*/
#include "HAL/HWTime/HWTime.h"

/*	SELF	*/
#include "HAL/UART/UART_Config.h"

#if uiCONF_UART_NUMBER_OF_NEEDED_UNITS > portUART_NUMBER_OF_UNITS

/*
 * Number of units to be created.
 */
#define uiNUMBER_OF_UNITS			(uiCONF_UART_NUMBER_OF_NEEDED_UNITS - portUART_NUMBER_OF_UNITS)

/*******************************************************************************
 * Structures:
 ******************************************************************************/
typedef struct{
	uint64_t ulTimestamp;
	uint8_t ucType;
}Rx_Transition_t;

typedef struct{
	SemaphoreHandle_t xUnitMutex;
	StaticSemaphore_t xUnitMutexStatic;

	QueueHandle_t xBitQueue;
	StaticQueue_t xBitQueueStatic;
	uint8_t pucBitQueueMemory[16 * sizeof(Rx_Transition_t)];

	uint64_t ulPrevDequeuedBitTimestamp;

	uint8_t ucOverrunErrorFlag : 1;
	uint8_t ucreceivedCompleteFrameFlag : 1;
	uint8_t ucSofDequeuedPreviously : 1;
}xHOS_UART_Unit_t;


/*******************************************************************************
 * Global and static variables:
 ******************************************************************************/
/*
 * Array of units.
 */
static xHOS_UART_Unit_t pxUnitArr[uiNUMBER_OF_UNITS];


/*******************************************************************************
 * Helping functions/macros:
 ******************************************************************************/
/*
 * Gets Rx port number of the given unit number.
 */
#define ucGET_RX_PORT(ucUnitNumber) (pucCONF_UART_SW_UNIT_RX_PORT_ARR[(ucUnitNumber)])

/*
 * Gets Rx pin number of the given unit number.
 */
#define ucGET_RX_PIN(ucUnitNumber) (pucCONF_UART_SW_UNIT_RX_PIN_ARR[(ucUnitNumber)])

/*
 * Gets unit's configured baudrate.
 */
#define uiGET_BAUDRATE(ucUnitNumber)	(puiCONF_UART_SW_UNIT_BAUDRATE_ARR[(ucUnitNumber)])

/*
 * Gets unit's bit time in microseconds.
 */
#define uiGET_BIT_TIME_US(ucUnitNumber)	(((1000000ul * 95) / 100) / uiGET_BAUDRATE(ucUnitNumber))

/*
 * Waits for a byte to be received in the given timeout. If successfully received,
 * byte is written to "*pcByte" and function returns 1. Otherwise, function returns 0.
 */
__attribute__((always_inline))
static inline uint8_t ucReadByte(uint8_t ucUnitNumber, int8_t* pcByte, TickType_t xTimeout)
{
	xHOS_UART_Unit_t* pxUnit = &pxUnitArr[ucUnitNumber];
	volatile Rx_Transition_t xTrans;
	uint8_t uc1stPropertyAcheived, uc2ndPropertyAcheived;

	uint8_t ucNBits, ucBitCount = 0;
	uint64_t ulDeltaTimeUs;
	uint16_t usFrame = 0;

	TickType_t xCurrentTime = xTaskGetTickCount();
	TickType_t xEndTime = xCurrentTime + xTimeout;

	if (xEndTime < xCurrentTime)	/*	If addition resulted in an overflow	*/
		xEndTime = portMAX_DELAY;

	/*
	 * Keep dequeuing from bit queue until SOF bit is detected, or given timeout
	 * passes.
	 *
	 * An SOF could be sensed by these properties:
	 * 	-	It may be a falling edge on the Rx pin after a while of being idle.
	 * 	-	It may be a falling edge on the Rx pin after having received a complete
	 * 		frame.
	 * 	-	It may be dequeued when reading previous frame.
	 */
	if (pxUnit->ucSofDequeuedPreviously)
	{
		pxUnit->ucSofDequeuedPreviously = 0;
	}

	else
	{
		while(1)
		{
			xCurrentTime = xTaskGetTickCount();
			if (xCurrentTime >= xEndTime)
				return 0;

			/*	Dequeue first of the queue	*/
			if (!xQueueReceive(pxUnit->xBitQueue, (void*)&xTrans, xEndTime-xCurrentTime))
				return 0;

			/*	If it is not a falling edge, ignore it and continue	*/
			if (xTrans.ucType != 0)
			{
				pxUnit->ulPrevDequeuedBitTimestamp = xTrans.ulTimestamp;
				continue;
			}

			/*
			 * Check 1st property (It comes after a while of being idle).
			 */
			uc1stPropertyAcheived =
				(	xTrans.ulTimestamp - pxUnit->ulPrevDequeuedBitTimestamp >=
					uiGET_BIT_TIME_US(ucUnitNumber)	);

			/*
			 * Check 2nd property (It comes after having received a complete frame).
			 */
			uc2ndPropertyAcheived = pxUnit->ucreceivedCompleteFrameFlag;

			/*
			 * update timestamp of previous dequeued transition.
			 */
			pxUnit->ulPrevDequeuedBitTimestamp = xTrans.ulTimestamp;

			/*	if any of the two properties is achieved, break	*/
			if (uc1stPropertyAcheived || uc2ndPropertyAcheived)
				break;
		}
	}

	/*	Clear Rx complete flag	*/
	pxUnit->ucreceivedCompleteFrameFlag = 0;

	/*	Convert transitions in the queue into usable stream of bits (SOF + Data + EOF)	*/
	while(ucBitCount < 1 + 8 + 1)
	{
		xCurrentTime = xTaskGetTickCount();
		if (xCurrentTime >= xEndTime)
			return 0;

		/*	Dequeue first of the queue (first; wait for time of one frame)	*/
		if (!xQueueReceive(pxUnit->xBitQueue, (void*)&xTrans, pdMS_TO_TICKS((15 * uiGET_BIT_TIME_US(ucUnitNumber)) / 1000)))
		{
			/*	If queue is empty, and having already received SOF, rest of the stream must be 1's	*/
			ucNBits = 10 - ucBitCount;
			usFrame |= (((1 << ucNBits) - 1) << ucBitCount);
			break;
		}

		/*	Calculate number of bits between this transition and the previous one	*/
		ulDeltaTimeUs = ulHOS_HWTime_TICKS_TO_US(xTrans.ulTimestamp - pxUnit->ulPrevDequeuedBitTimestamp);
		ucNBits = ulDeltaTimeUs / uiGET_BIT_TIME_US(ucUnitNumber);

		/*	Update timestamp	*/
		pxUnit->ulPrevDequeuedBitTimestamp = xTrans.ulTimestamp;

		/*	If last transition is falling, set "ucNBits" bits starting from "ucBitCount"		 */
		if (xTrans.ucType == 0)
			usFrame |= (((1 << ucNBits) - 1) << ucBitCount);

		/*
		 * Otherwise, if last transition is rising, clear "ucNBits" bits starting
		 * from "ucBitCount". (Actually, they are cleared by default)
		 */
		else
		{

		}

		/*	Increment bit count	*/
		ucBitCount += ucNBits;
	}

	/*	Check EOF	*/
	if (usFrame >> 9 != 1)
		return 0;

	/*	Write data	*/
	*pcByte = (int8_t)((usFrame >> 1) & 0xFF);

	/*	Set Rx complete flag	*/
	pxUnit->ucreceivedCompleteFrameFlag = 1;

	/*	Check whether SOF of next frame is dequeued in this frame or not	*/
	if (usFrame >> 10 == 0 && ucBitCount >= 10)
		pxUnit->ucSofDequeuedPreviously = 1;

	return 1;
}


/*******************************************************************************
 * Callbacks:
 ******************************************************************************/
static void vRxExtiCallback(void* pvParams)
{
	uint8_t ucUnitNumber = (uint32_t)pvParams;
	xHOS_UART_Unit_t* pxUnit = &pxUnitArr[ucUnitNumber];
	Rx_Transition_t xTrans, xFooTrans;

	/*	Get timestamp	*/
	xTrans.ulTimestamp = ulHOS_HWTime_getTimestampFromISR();

	/*	Get edge type	*/
	xTrans.ucType = ucPORT_DIO_READ_PIN(	ucGET_RX_PORT(ucUnitNumber),
											ucGET_RX_PIN (ucUnitNumber)	);

	/*	Enqueue transition	*/
	if (!xQueueSendFromISR(pxUnit->xBitQueue, (void*)&xTrans, NULL))
	{
		/*
		 * If bit queue is full, raise overrun error flag, delete first of the
		 * queue, and queue new transition.
		 */
		pxUnit->ucOverrunErrorFlag = 1;
		xQueueReceiveFromISR(pxUnit->xBitQueue, (void*)&xFooTrans, NULL);
		xQueueSendFromISR(pxUnit->xBitQueue, (void*)&xTrans, NULL);
	}
}

/*******************************************************************************
 * Driver functions:
 ******************************************************************************/
/*
 * Initializes all SW based UART units.
 *
 * Notes:
 * 		-	HWTime must be initialized first.
 */
void vHOS_UART_SW_init(void)
{
	xHOS_UART_Unit_t* pxUnit;

	for (uint32_t i = 0; i < uiNUMBER_OF_UNITS; i++)
	{
		pxUnit = &pxUnitArr[i];

		/*	create unit's mutex	*/
		pxUnit->xUnitMutex =
			xSemaphoreCreateMutexStatic(&pxUnit->xUnitMutexStatic);
		xSemaphoreGive(pxUnit->xUnitMutex);

		/*	Create Bit queue	*/
		pxUnit->xBitQueue = xQueueCreateStatic(	16,
												sizeof(Rx_Transition_t),
												pxUnit->pucBitQueueMemory,
												&pxUnit->xBitQueueStatic	);

		pxUnit->ulPrevDequeuedBitTimestamp = 0;

		pxUnit->ucOverrunErrorFlag = 0;

		pxUnit->ucreceivedCompleteFrameFlag = 1;

		pxUnit->ucSofDequeuedPreviously = 0;

		/*	Initialize unit's Rx pin as an EXTI channel	*/
		vPort_DIO_initPinInput(ucGET_RX_PORT(i), ucGET_RX_PIN(i), 0);

		vPort_EXTI_setEdge(ucGET_RX_PORT(i), ucGET_RX_PIN(i), 2);

		vPort_EXTI_setCallback(	ucGET_RX_PORT(i),
								ucGET_RX_PIN(i),
								vRxExtiCallback,
								(void*)i	);

		vPORT_EXTI_ENABLE_LINE(ucGET_RX_PORT(i), ucGET_RX_PIN(i));

		/*	Initialize unit's Rx EXTI channel in interrupt controller	*/
		uint8_t ucIrqNum = uiPort_EXTI_getIrqNum(ucGET_RX_PORT(i), ucGET_RX_PIN(i));
		VPORT_INTERRUPT_SET_PRIORITY(ucIrqNum, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 2);
		vPORT_INTERRUPT_ENABLE_IRQ(ucIrqNum);
	}
}

/*
 * Sends data using a SW unit.
 */
void vHOS_UART_SW_send(uint8_t ucUnitNumber, int8_t* pcArr, uint32_t uiSize)
{
	/*	TODO	*/
}

/*
 * Receives data using a SW unit.
 *
 * Notes:
 * 		-	In order for the receive operation to be performed properly (i.e.:
 * 			without overrun error), this function should be called in a high
 * 			priority task. If the application task must be of low priority, then
 * 			a seperate task should be created for receiving.
 */
uint8_t ucHOS_UART_SW_receive(		uint8_t ucUnitNumber,
									int8_t* pcInArr,
									uint32_t uiSize,
									TickType_t xTimeout	)
{
	TickType_t xCurrentTime = xTaskGetTickCount();
	TickType_t xEndTime = xCurrentTime + xTimeout;

	if (xEndTime < xCurrentTime)	/*	If addition resulted in an overflow	*/
		xEndTime = portMAX_DELAY;

	for (uint32_t i = 0; i < uiSize; i++)
	{
		/*	If timeout has passed	*/
		if (xCurrentTime >= xEndTime)
			return 0;

		/*	Read byte, if failed, return 0	*/
		if (!ucReadByte(ucUnitNumber, &pcInArr[i], xEndTime - xCurrentTime))
			return 0;

		/*	Update current time	*/
		xCurrentTime = xTaskGetTickCount();
	}

	return 1;
}

/*
 * Locks a SW unit.
 */
uint8_t ucHOS_UART_SW_takeMutex(uint8_t ucUnitNumber, TickType_t xTimeout)
{
	return xSemaphoreTake(pxUnitArr[ucUnitNumber].xUnitMutex, xTimeout);
}

/*
 * Unlocks a SW unit.
 */
void vHOS_UART_SW_releaseMutex(uint8_t ucUnitNumber)
{
	xSemaphoreGive(pxUnitArr[ucUnitNumber].xUnitMutex);
}

/*
 * Blocks untill end of current transmission.
 */
uint8_t ucHOS_UART_SW_blockUntilTransmissionComplete(uint8_t ucUnitNumber, TickType_t xTimeout)
{
	/*	TODO	*/
}








#endif











