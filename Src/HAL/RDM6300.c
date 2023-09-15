/*
 * RDM6300.c
 *
 *  Created on: Sep 15, 2023
 *      Author: Ali Emad
 */

/*	LIB	*/
#include <stdint.h>
#include <stdio.h>
#include "LIB/CheckSum.h"

/*	FreeRTOS	*/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/*	MCAL (Ported)	*/
#include "MCAL_Port/Port_DIO.h"
#include "MCAL_Port/Port_EXTI.h"
#include "MCAL_Port/Port_Interrupt.h"

/*	COTS-OS	*/
#include "HAL/HAL_OS.h"
#include "RTOS_PRI_Config.h"

/*	SELF	*/
#include "HAL/RDM6300/RDM6300.h"

/*******************************************************************************
 * Helping functions/macros.
 ******************************************************************************/
static inline void vBlockUntilIntTrigger(xHOS_RDM6300_t* pxHandle)
{
	/*	Assure new read semaphore is not available	*/
	xSemaphoreTake(pxHandle->xNewReadSemaphore, 0);

	/*	Enable interrupt (Disabled in the ISR)	*/
	vPort_EXTI_disableLine(pxHandle->ucIntPort, pxHandle->ucIntPin);

	/*	Block on the new read semaphore	*/
	xSemaphoreTake(pxHandle->xNewReadSemaphore, portMAX_DELAY);
}

static inline uint8_t ucTryReadingNewFrame(xHOS_RDM6300_t* pxHandle)
{
	return ucHOS_UART_receive(
				pxHandle->ucUartUnitNumber,
				(int8_t*)&pxHandle->xFrame,
				sizeof(xHOS_RDM6300_Frame_t),
				pdMS_TO_TICKS(15)	); // transmission time of 13-bytes < 15ms
}

static inline void vEnqueueNewReading(xHOS_RDM6300_t* pxHandle)
{
	/*	If there's no available space, overwrite the queue	*/
	if (uxQueueSpacesAvailable(pxHandle->xReadQueue) == 0)
		xQueueOverwrite(pxHandle->xReadQueue, (void*)pxHandle->xFrame.xID.pucData);

	/*	Otherwise, normal enqueue	*/
	else
		xQueueSend(pxHandle->xReadQueue, (void*)pxHandle->xFrame.xID.pucData, portMAX_DELAY);
}

static inline void vProcessNewFrame(xHOS_RDM6300_t* pxHandle)
{
	/*	Check SOF	*/
	if (pxHandle->xFrame.ucSOF != 0x02)
		return;

	/*	Check EOF	*/
	if (pxHandle->xFrame.ucEOF != 0x03)
		return;

	/*	Calculate checksum	*/
	uint8_t ucCheckSum = ucLIB_CheckSum_get8BitCheck(pxHandle->xFrame.xID.pucData, 10);

	/*	If check-sum matches, add new ID to the queue	*/
	if (ucCheckSum == pxHandle->xFrame.ucCheckSum)
		vEnqueueNewReading(pxHandle);
}

/*******************************************************************************
 * ISR callback:
 ******************************************************************************/
static void vCallback(void* pvParams)
{
	xHOS_RDM6300_t* pxHandle = (xHOS_RDM6300_t*)pvParams;

	/*	Validate interrupt's signal	*/
	if (ucPort_DIO_readPin(pxHandle->ucIntPort, pxHandle->ucIntPin) == 1)
		return;

	/*	Disable interrupt	*/
	vPort_EXTI_disableLine(pxHandle->ucIntPort, pxHandle->ucIntPin);

	/*	Release the new read semaphore	*/
	BaseType_t xHPTWoken = pdFALSE;
	xSemaphoreGiveFromISR(pxHandle->xNewReadSemaphore, &xHPTWoken);
	portYIELD_FROM_ISR(xHPTWoken);
}

/*******************************************************************************
 * RTOS Task code:
 ******************************************************************************/
static void vTask(void* pvParams)
{
	xHOS_RDM6300_t* pxHandle = (xHOS_RDM6300_t*)pvParams;

	while(1)
	{
		/*	Block until INT pin is triggered	*/
		vBlockUntilIntTrigger(pxHandle);

		/*	Try reading a frame over the UART bus. If read fails, continue	*/
		if (!ucTryReadingNewFrame(pxHandle))
			continue;

		/*	Otherwise, process the new read frame	*/
		vProcessNewFrame(pxHandle);
	}
}

/*******************************************************************************
 * API functions:
 ******************************************************************************/

/*
 * See header for info.
 */
void vHOS_RDM6300_init(xHOS_RDM6300_t* pxHandle)
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

	/*	Create queue handle	*/
	pxHandle->xReadQueue = xQueueCreateStatic(
		uiCONF_RDM6300_READ_QUEUE_LEN,
		sizeof(xHOS_RDM6300_Frame_t),
		(uint8_t*)pxHandle->pucReadQueueMemory,
		&pxHandle->xReadQueueStatic	);

	/*	Initialize EXTI channel	*/
	vPort_DIO_initPinInput(pxHandle->ucIntPort, pxHandle->ucIntPin, 0);

	vPort_EXTI_disableLine(pxHandle->ucIntPort, pxHandle->ucIntPin);

	vPort_EXTI_initLine(pxHandle->ucIntPort, pxHandle->ucIntPin, 0);

	vPort_EXTI_setCallback(	pxHandle->ucIntPort,
							pxHandle->ucIntPin,
							vCallback,
							(void*)pxHandle	);

	/*	Initialize interrupt controller	*/
	vPort_Interrupt_setPriority(
		pxPortInterruptExtiIrqNumberArr[pxHandle->ucIntPin],
		configLIBRARY_LOWEST_INTERRUPT_PRIORITY	);

	vPort_Interrupt_enableIRQ(pxPortInterruptExtiIrqNumberArr[pxHandle->ucIntPin]);
}

/*
 * Gets new reading (ID of newly scanned tag).
 *
 * Notes:
 * 		-	If a new reading was previously received, or was received within the
 * 			given timeout, the ID is copied to "pxID" and function returns 1.
 * 			Otherwise, it returns 0.
 */
uint8_t ucHOS_RDM6300_getNewReading(	xHOS_RDM6300_t* pxHandle,
										xHOS_RDM6300_ID_t* pxID,
										TickType_t xTimeout)
{
	return xQueueReceive(pxHandle->xReadQueue, (void*)pxID, xTimeout);
}















