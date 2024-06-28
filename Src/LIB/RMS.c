/*
 * RMS.c
 *
 *  Created on: Jun 15, 2024
 *      Author: Ali Emad
 */

/*	LIB	*/
#include "stdint.h"
#include "math.h"

/*	RTOS	*/
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

/*	SELF	*/
#include "LIB/RMS/RMS.h"


/*
 * See header file for info.
 */
void vLIB_RMS_init(	xLIB_RMS_t* pxHandle,
					float* piDataArr,
					uint32_t uiN	)
{
	/*	Create queue handle	*/
	pxHandle->xQueue = xQueueCreateStatic(	uiN,
	                                 	 	sizeof(float),
											(uint8_t*)piDataArr,
											&pxHandle->xQueueStatic	);

	/*	Fill the queue with "uiN" zero samples	*/
	float fZero = 0;
	for (uint32_t i = 0; i < uiN-2; i++)
	{
		xQueueSend(pxHandle->xQueue, (void*)&fZero, 0);
	}

	pxHandle->fX0 = 0;
	pxHandle->fXN_1 = 0;
	pxHandle->fSum = 0;

	/*	Copy "uiN" to the handle	*/
	pxHandle->uiN = uiN;

	/*	Initialize mutex	*/
	pxHandle->xMutex = xSemaphoreCreateMutexStatic(&pxHandle->xMutexStatic);
	xSemaphoreGive(pxHandle->xMutex);
}

/*
 * See header file for info.
 */
uint8_t ucLIB_RMS_lock(xLIB_RMS_t* pxHandle, TickType_t xTimeout)
{
	return xSemaphoreTake(pxHandle->xMutex, xTimeout);
}

/*
 * See header file for info.
 */
vLIB_RMS_unlock(xLIB_RMS_t* pxHandle)
{
	xSemaphoreGive(pxHandle->xMutex);
}

/*
 * See header file for info.
 */
void vLIB_RMS_update(xLIB_RMS_t* pxHandle, float fNewVal)
{
	/*	Read then remove front of the queue (current before last sample)	*/
	float fFront;
	xQueueReceive(pxHandle->xQueue, (void*)&fFront, 0);

	/*	Subtract front of the queue from the sum	*/
	pxHandle->fSum -= fFront;

	/*	update current last sample with the current before last sample	*/
	pxHandle->fXN_1 = fFront;

	/*	Add current first sample to the sum	*/
	pxHandle->fSum += pxHandle->fX0;

	/*	Add current first sample to back of the queue	*/
	xQueueSend(pxHandle->xQueue, (void*)&pxHandle->fX0, 0);

	/*	Write new value to current first sample	*/
	pxHandle->fX0 = fNewVal * fNewVal;
}

float fLIB_RMS_getValue(xLIB_RMS_t* pxHandle)
{
	return sqrt((pxHandle->fX0 + pxHandle->fXN_1 + 2.0f * pxHandle->fSum) / (2.0f * (float)pxHandle->uiN));
}








