/*
 * DMA.c
 *
 *  Created on: Aug 23, 2023
 *      Author: Ali Emad
 */

/*	LIB	*/
#include <stdint.h>
#include "LIB/Assert.h"

/*	FreeRTOS	*/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/*	MCAL (Ported)	*/
#include "MCAL_Port/Port_DMA.h"
#include "MCAL_Port/Port_Interrupt.h"

/*	SELF	*/
#include "HAL/DMA/DMA.h"

#if portDMA_IS_AVAILABLE

/*******************************************************************************
 * Helping structures:
 ******************************************************************************/
typedef struct{
	uint8_t ucUnitNumber;
	uint8_t ucChannelNumber;

	uint8_t ucIsInQueue;

	SemaphoreHandle_t xMutex;
	StaticSemaphore_t xMutexStatic;

	SemaphoreHandle_t xTransferCompleteSemaphore;
	StaticSemaphore_t xTransferCompleteSemaphoreStatic;
}xHOS_DMA_Channel_t;

/*******************************************************************************
 * Helping functions / macros:
 ******************************************************************************/
#define uiNUMBER_OF_CHANNELS	(portDMA_NUMBER_OF_UNITS * portDMA_NUMBER_OF_CHANNELS_PER_UNIT)

/*******************************************************************************
 * Static data:
 ******************************************************************************/
static xHOS_DMA_Channel_t pxChannelArr[uiNUMBER_OF_CHANNELS];

static QueueHandle_t xChannelQueue;
static StaticQueue_t xChannelQueueStatic;
static uint8_t pucChannelQueueStorage[uiNUMBER_OF_CHANNELS * sizeof(xHOS_DMA_Channel_t*)];

/*
 * This extra mutex is mandatory to have the ability of editing
 * "pxChannel->ucIsInQueue" right after receiving from the queue as a single,
 * thread-safe operation.
 */
static SemaphoreHandle_t xQueueMutex;
static StaticSemaphore_t xQueueMutexStatic;

/*******************************************************************************
 * ISR callback:
 ******************************************************************************/
static void vCallback(void* pvParams)
{
	xHOS_DMA_Channel_t* pxChannel = (xHOS_DMA_Channel_t*)pvParams;
	BaseType_t xHighPriorityTaskWoken = pdFALSE;

	xSemaphoreGiveFromISR(	pxChannel->xTransferCompleteSemaphore,
							&xHighPriorityTaskWoken	);

	portYIELD_FROM_ISR(xHighPriorityTaskWoken);
}

/*******************************************************************************
 * API functions:
 ******************************************************************************/
/*
 * See header for info.
 */
void vHOS_DMA_init(void)
{
	xHOS_DMA_Channel_t* pxChannel;

	/*	Initialize channels (pointers) queue	*/
	xChannelQueue = xQueueCreateStatic(	uiNUMBER_OF_CHANNELS,
										sizeof(xHOS_DMA_Channel_t*),
										pucChannelQueueStorage,
										&xChannelQueueStatic	);

	xQueueMutex = xSemaphoreCreateMutexStatic(&xQueueMutexStatic);
	xSemaphoreGive(xQueueMutex);

	uint32_t i = 0;
	for (uint8_t ucUnit = 0; ucUnit < portDMA_NUMBER_OF_UNITS; ucUnit++)
	{
		for (uint8_t ucCh = 0; ucCh < portDMA_NUMBER_OF_CHANNELS_PER_UNIT; ucCh++)
		{
			/*	Initialize channel in channels array	*/
			pxChannelArr[i].ucUnitNumber = ucUnit;
			pxChannelArr[i].ucChannelNumber = ucCh;

			pxChannelArr[i].xMutex =
				xSemaphoreCreateMutexStatic(&pxChannelArr[i].xMutexStatic);
			xSemaphoreGive(pxChannelArr[i].xMutex);

			pxChannelArr[i].xTransferCompleteSemaphore =
				xSemaphoreCreateBinaryStatic(
					&pxChannelArr[i].xTransferCompleteSemaphoreStatic	);
			xSemaphoreGive(pxChannelArr[i].xTransferCompleteSemaphore);

			/*	Enqueue pointer to the channel to channels' queue	*/
			pxChannel = &pxChannelArr[i];
			xQueueSend(xChannelQueue, (void*)&pxChannel, portMAX_DELAY); // shall never wait.
			pxChannelArr[i].ucIsInQueue = 1;

			/*	Initialize channel's interrupt	*/
			vPort_Interrupt_setPriority(	pxPortInterruptDmaIrqNumberArr[i],
											configLIBRARY_LOWEST_INTERRUPT_PRIORITY	);

			vPort_Interrupt_enableIRQ(pxPortInterruptDmaIrqNumberArr[i]);

			vPort_DMA_setTransferCompleteCallback(	ucUnit,
													ucCh,
													vCallback,
													(void*)pxChannel	);

			vPort_DMA_enableTransferCompleteInterrupt(ucUnit, ucCh);

			i++;
		}
	}
}

/*
 * See header for info.
 */
uint8_t ucHOS_DMA_lockAnyChannel(	uint8_t* pucUnitNumber,
									uint8_t* pucChannelNumber,
									TickType_t xTimeout	)
{
	xHOS_DMA_Channel_t* pxChannel;
	uint8_t ucSuccessful;

	TickType_t xCurrentTime = xTaskGetTickCount();
	TickType_t xEndTime;
	if (xTimeout == portMAX_DELAY)
		xEndTime = portMAX_DELAY;
	else
		xEndTime = xCurrentTime + xTimeout;

	while(xCurrentTime <= xEndTime)
	{
		/*	Lock the queue for this task.	*/
		ucSuccessful = xSemaphoreTake(xQueueMutex, xEndTime - xCurrentTime);
		if (!ucSuccessful)
			return 0;

		/*	Try to dequeue first of the channel queue	*/
		ucSuccessful = xQueueReceive(	xChannelQueue,
										(void*)&pxChannel,
										xEndTime - xCurrentTime	);
		if (!ucSuccessful)
			return 0;

		/*
		 * If successfully dequeue, label the channel as "not in the queue".
		 */
		pxChannel->ucIsInQueue = 0;

		/*	Release queue lock	*/
		xSemaphoreGive(xQueueMutex);

		/*	If channel is available, return successful with it	*/
		ucSuccessful = xSemaphoreTake(pxChannel->xMutex, 0);
		if (ucSuccessful)
		{
			*pucUnitNumber = pxChannel->ucUnitNumber;
			*pucChannelNumber = pxChannel->ucChannelNumber;
			return 1;
		}

		xCurrentTime = xTaskGetTickCount();
	}

	return 0;
}

/*
 * See header for info.
 */
uint8_t ucHOS_DMA_lockChannel(	uint8_t ucUnitNumber,
								uint8_t ucChannelNumber,
								TickType_t xTimeout	)
{
	uint32_t uiIndex = ucUnitNumber * portDMA_NUMBER_OF_CHANNELS_PER_UNIT + ucChannelNumber;
	return xSemaphoreTake(pxChannelArr[uiIndex].xMutex, xTimeout);
	/*
	 * Channel needn't be dequeued from the queue, as it is in-efficient to dequeue
	 * from middle of the queue. Implementation of both "ucHOS_DMA_lockAnyChannel()"
	 * and "ucHOS_DMA_releaseChannel()" helps achieving this.
	 */
}

/*
 * See header for info.
 */
__attribute__((always_inline)) inline
void vHOS_DMA_startTransfer(xHOS_DMA_TransInfo_t* pxInfo)
{
	vPort_DMA_startTransfer(pxInfo);
}

/*
 * See header for info.
 */
uint8_t ucHOS_DMA_blockUntilTransferComplete(	uint8_t ucUnitNumber,
												uint8_t ucChannelNumber,
												TickType_t xTimeout	)
{
	uint32_t uiIndex = ucUnitNumber * portDMA_NUMBER_OF_CHANNELS_PER_UNIT + ucChannelNumber;

	return xSemaphoreTake(	pxChannelArr[uiIndex].xTransferCompleteSemaphore,
							xTimeout	);
}

/*
 * See header for info.
 */
void vHOS_DMA_clearTransferCompleteFlag(	uint8_t ucUnitNumber,
											uint8_t ucChannelNumber	)
{
	uint32_t uiIndex = ucUnitNumber * portDMA_NUMBER_OF_CHANNELS_PER_UNIT + ucChannelNumber;

	xSemaphoreTake(pxChannelArr[uiIndex].xTransferCompleteSemaphore, 0);
}

/*
 * See header for info.
 */
uint8_t ucHOS_DMA_releaseChannel(	uint8_t ucUnitNumber,
								uint8_t ucChannelNumber,
								TickType_t xTimeout	)
{
	uint8_t ucSuccessful;

	uint32_t uiIndex = ucUnitNumber * portDMA_NUMBER_OF_CHANNELS_PER_UNIT + ucChannelNumber;
	xHOS_DMA_Channel_t* pxChannel = &pxChannelArr[uiIndex];

	/*	Lock the queue for this task.	*/
	ucSuccessful = xSemaphoreTake(xQueueMutex, xTimeout);
	if (!ucSuccessful)
		return 0;

	/*	Release mutex of this channel	*/
	xSemaphoreGive(pxChannel->xMutex);

	/*	If not in the queue, enqueue this channel and label it as "in the queue"	*/
	if (pxChannel->ucIsInQueue == 0)
	{
		vLib_ASSERT(xQueueSend(xChannelQueue, (void*)&pxChannel, 0), 1); // shall never fail.
		pxChannel->ucIsInQueue = 1;
	}

	/*	release queue's mutex	*/
	xSemaphoreGive(xQueueMutex);

	return 1;
}












#endif		/*		portDMA_IS_AVAILABLE		*/














