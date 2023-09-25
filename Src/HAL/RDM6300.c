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
#include "LIB/String.h"

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

	/*	Clear EXTI pending flag	*/
	vPORT_EXTI_CLEAR_PENDING_FLAG(pxHandle->ucIntPort, pxHandle->ucIntPin);

	/*	Enable interrupt (Disabled in the ISR)	*/
	vPORT_EXTI_ENABLE_LINE(pxHandle->ucIntPort, pxHandle->ucIntPin);

	/*	Block on the new read semaphore	*/
	xSemaphoreTake(pxHandle->xNewReadSemaphore, portMAX_DELAY);
}

static inline uint8_t ucTryReadingNewFrame(xHOS_RDM6300_t* pxHandle)
{
	return ucHOS_UART_receive(
				pxHandle->ucUartUnitNumber,
				(int8_t*)&pxHandle->xFrame,
				sizeof(xHOS_RDM6300_Frame_t),
				pdMS_TO_TICKS(150)	); // transmission time of 13-bytes < 15ms
}

static inline void vEnqueueNewReading(xHOS_RDM6300_t* pxHandle)
{
	/*	If there's no available space, overwrite the queue	*/
	if (uxQueueSpacesAvailable(pxHandle->xReadQueue) == 0)
		xQueueOverwrite(pxHandle->xReadQueue, (void*)pxHandle->xTempID.pucData);

	/*	Otherwise, normal enqueue	*/
	else
		xQueueSend(pxHandle->xReadQueue, (void*)&pxHandle->xTempID, portMAX_DELAY);
}

static inline void vProcessNewFrame(xHOS_RDM6300_t* pxHandle)
{
	uint8_t ucCheckSumReceived, ucCheckSumCalculated;

	/*	Check SOF	*/
	if (pxHandle->xFrame.ucSOF != 0x02)
		return;

	/*	Check EOF	*/
	if (pxHandle->xFrame.ucEOF != 0x03)
		return;

	/*	Calculate checksum	*/
	vLIB_String_str2hex((char*)pxHandle->xFrame.pcIDStr, pxHandle->xTempID.pucData, 5);
	ucCheckSumCalculated = ucLIB_CheckSum_get8BitCheck(pxHandle->xTempID.pucData, 5);

	vLIB_String_str2hex((char*)pxHandle->xFrame.pcCheckSumStr, &ucCheckSumReceived, 1);

	/*	If check-sum matches, add new ID to the queue	*/
	if (ucCheckSumCalculated == ucCheckSumReceived)
		vEnqueueNewReading(pxHandle);
}

/*******************************************************************************
 * ISR callback:
 ******************************************************************************/
static void vCallback(void* pvParams)
{
	xHOS_RDM6300_t* pxHandle = (xHOS_RDM6300_t*)pvParams;

	/*	Validate interrupt's signal	*/
	if (ucPORT_DIO_READ_PIN(pxHandle->ucIntPort, pxHandle->ucIntPin) == 0)
		return;

	/*	Disable interrupt	*/
	vPORT_EXTI_DISABLE_LINE(pxHandle->ucIntPort, pxHandle->ucIntPin);

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

	TickType_t xLastWakeTime = xTaskGetTickCount();

	const uint32_t uiDelayMs = (sizeof(xHOS_RDM6300_Frame_t) * 8000) / 9600;

	while(1)
	{
		/*	Block until INT pin is triggered	*/
		vBlockUntilIntTrigger(pxHandle);

		xLastWakeTime = xTaskGetTickCount();

		/*	Try reading a frame over the UART bus. If read fails, do nothing	*/
		if (!ucTryReadingNewFrame(pxHandle))
		{	}

		/*	Otherwise, process the new read frame	*/
		else
			vProcessNewFrame(pxHandle);

		/*	Block until next reading time	*/

		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(uiDelayMs));
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
		sizeof(xHOS_RDM6300_ID_t),
		(uint8_t*)pxHandle->pucReadQueueMemory,
		&pxHandle->xReadQueueStatic	);

	/*	Initialize EXTI channel	*/
	vPort_DIO_initPinInput(pxHandle->ucIntPort, pxHandle->ucIntPin, 0);

	vPORT_EXTI_DISABLE_LINE(pxHandle->ucIntPort, pxHandle->ucIntPin);

	vPort_EXTI_setEdge(pxHandle->ucIntPort, pxHandle->ucIntPin, 1);

	vPort_EXTI_setCallback(	pxHandle->ucIntPort,
							pxHandle->ucIntPin,
							vCallback,
							(void*)pxHandle	);

	/*	Initialize interrupt controller	*/
	VPORT_INTERRUPT_SET_PRIORITY(
		pxPortInterruptExtiIrqNumberArr[pxHandle->ucIntPin],
		configLIBRARY_LOWEST_INTERRUPT_PRIORITY	);

	vPORT_INTERRUPT_ENABLE_IRQ(pxPortInterruptExtiIrqNumberArr[pxHandle->ucIntPin]);
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















