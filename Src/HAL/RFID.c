/*
 * RFID.c
 *
 *  Created on: Sep 15, 2023
 *      Author: Ali Emad
 */

/*	LIB	*/
#include <stdint.h>
#include <stdio.h>
#include "LIB/CheckSum.h"
#include "LIB/String.h"

/*	FreeRTOS	*/

/*	MCAL (Ported)	*/
#include "MCAL_Port/Port_DIO.h"
#include "MCAL_Port/Port_EXTI.h"
#include "MCAL_Port/Port_Interrupt.h"

/*	COTS-OS	*/
#include "HAL/HAL_OS.h"
#include "RTOS_PRI_Config.h"

/*	SELF	*/
#include "HAL/RFID/RFID.h"

/*******************************************************************************
 * Helping functions/macros.
 ******************************************************************************/
#define _SOF	0x02
#define _EOF	0x03

static inline uint8_t ucCheckFrameSof(xHOS_RFID_t* pxHandle)
{
	int8_t cTempByte;

	/*	If queue is empty, check fails	*/
	if (!xQueuePeek(pxHandle->xTempFrameQueue, (void*)&cTempByte, 0))
		return 0;

	/*	If queue's front is not equal to SOF, check fails	*/
	if (cTempByte != _SOF)
		return 0;

	/*	Otherwise, check is successful	*/
	return 1;
}

/*
 * This function assumes that check sum is currently first of the temp queue.
 */
static inline uint8_t ucGetReceivedChecksum(xHOS_RFID_t* pxHandle)
{
	uint8_t ucCheckSumReceived;

	/*	Dequeue received checksum	*/
#ifdef uiCONF_RFID_TYPE_RDM6300
	/*	TODO: dequeue two chars, parse them into one byte, this is the received checksum	*/
#endif

#ifdef uiCONF_RFID_TYPE_UNKNOWN_125KHZ
	xQueueReceive(pxHandle->xTempFrameQueue, (void*)&ucCheckSumReceived, portMAX_DELAY);
#endif

	return ucCheckSumReceived;
}

static inline void vEnqueueNewID(xHOS_RFID_t* pxHandle)
{

}

/*******************************************************************************
 * ISR callback:
 ******************************************************************************/


/*******************************************************************************
 * RTOS Task code:
 ******************************************************************************/
static void vTask(void* pvParams)
{
	xHOS_RFID_t* pxHandle = (xHOS_RFID_t*)pvParams;

	TickType_t xSofLastTimestamp = xTaskGetTickCount();

	int8_t cNewByte, cFooByte;

	char pcIDStr[10];

	uint8_t ucCheckSumCalculated, ucCheckSumReceived;

	while(1)
	{
		/*	Block until new byte is received	*/
		ucHOS_UART_receive(	pxHandle->ucUartUnitNumber,
							&cNewByte,
							1,
							portMAX_DELAY	);

		/*	Push byte to temporary frame queue	*/
		if (uxQueueSpacesAvailable(pxHandle->xTempFrameQueue) == 0)
			xQueueReceive(pxHandle->xTempFrameQueue, (void*)&cFooByte, 0);
		xQueueSend(pxHandle->xTempFrameQueue, (void*)&cNewByte, 0);

		/*	If byte is equal to SOF, update SOF timestamp	*/
		if (cNewByte == _SOF)
			xSofLastTimestamp = xTaskGetTickCount();

		/*
		 * Otherwise, if byte is equal to to EOF, ther's a chance of having a completed
		 * frame.
		 */
		else if (cNewByte == _EOF)
		{
			/*
			 * If frame's SOF is not in its place yet, continue in the loop waiting
			 * for next byte
			 */
			if (!ucCheckFrameSof(pxHandle))
				continue;

			/*
			 * If time since last SOF is larger than 100ms, then the data is of two
			 * independent frames, it just happens to have bytes similar to SOF & EOF
			 * at their places (relatively). Hence, continue in the loop waiting for
			 * next byte.
			 */
			if (xTaskGetTickCount() - xSofLastTimestamp > pdMS_TO_TICKS(100))
				continue;

			/*
			 * If frame is successfully received, dequeue the ID field into a char
			 * array (as orientation in the queue is not always constant).
			 */
			for (uint8_t i = 0; i < 10; i++)
				xQueueReceive(pxHandle->xTempFrameQueue, (void*)pcIDStr, portMAX_DELAY);

			/*	Parse this char array into a byte array	*/
			vLIB_String_str2hex(pcIDStr, pxHandle->xTempID.pucData, 5);

			/*	Calculate checksum of this byte array.	*/
			ucCheckSumCalculated = ucLIB_CheckSum_get8BitCheck(pxHandle->xTempID.pucData, 5);

			/*	Get the received checksum	*/
			ucCheckSumReceived = ucGetReceivedChecksum(pxHandle);

			/*
			 * If checksum is not valid, continue in the loop waiting for next byte.
			 */
			if (ucCheckSumReceived != ucCheckSumCalculated)
				continue;

			/*
			 * Otherwise, if checksum is valid, enqueue new ID to IDs queue.
			 */

			/*	If no space is available, dequeue first item into a non used location	*/
			if (uxQueueSpacesAvailable(pxHandle->xReadQueue) == 0)
				xQueueReceive(pxHandle->xReadQueue, (void*)pcIDStr, portMAX_DELAY);

			xQueueSend(pxHandle->xReadQueue, (void*)pxHandle->xTempID.pucData, portMAX_DELAY);

		}
	}
}

/*******************************************************************************
 * API functions:
 ******************************************************************************/

/*
 * See header for info.
 */
void vHOS_RFID_init(xHOS_RFID_t* pxHandle)
{
	/*	Initialize task	*/
	static uint8_t ucCreatedObjectsCount = 0;
	char pcTaskName[configMAX_TASK_NAME_LEN];
	sprintf(pcTaskName, "RDM%d", ucCreatedObjectsCount++);

	pxHandle->xTask = xTaskCreateStatic(	vTask,
											pcTaskName,
											configMINIMAL_STACK_SIZE,
											(void*)pxHandle,
											configHOS_HARD_REAL_TIME_TASK_PRI,
											pxHandle->pxTaskStack,
											&pxHandle->xTaskStatic	);

	/*	Initialize new read semaphore	*/
	pxHandle->xNewReadSemaphore =
		xSemaphoreCreateBinaryStatic(&pxHandle->xNewReadSemaphoreStatic);
	xSemaphoreTake(pxHandle->xNewReadSemaphore, 0);

	/*	Create IDs queue handle	*/
	pxHandle->xReadQueue = xQueueCreateStatic(
		uiCONF_RFID_READ_QUEUE_LEN,
		sizeof(xHOS_RFID_ID_t),
		(uint8_t*)pxHandle->pucReadQueueMemory,
		&pxHandle->xReadQueueStatic	);

	/*	Create temporary frame queue handle	*/
	pxHandle->xTempFrameQueue = xQueueCreateStatic(
		sizeof(xHOS_RFID_Frame_t),
		1,
		(uint8_t*)pxHandle->pucTempFrameQueueMemory,
		&pxHandle->xTempFrameQueueStatic	);
}

/*
 * Gets new reading (ID of newly scanned tag).
 *
 * Notes:
 * 		-	If a new reading was previously received, or was received within the
 * 			given timeout, the ID is copied to "pxID" and function returns 1.
 * 			Otherwise, it returns 0.
 */
uint8_t ucHOS_RFID_getNewReading(	xHOS_RFID_t* pxHandle,
									xHOS_RFID_ID_t* pxID,
									TickType_t xTimeout)
{
	return xQueueReceive(pxHandle->xReadQueue, (void*)pxID, xTimeout);
}














