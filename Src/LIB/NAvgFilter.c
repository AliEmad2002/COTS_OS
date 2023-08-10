/*
 * NAvgFilter.c
 *
 *  Created on: Aug 9, 2023
 *      Author: Ali Emad
 */

/*	LIB	*/
#include "stdint.h"

/*	RTOS	*/
#include "FreeRTOS.h"
#include "queue.h"

/*	SELF	*/
#include "LIB/NAvgFilter/NAvgFilter.h"


/*
 * See header file for info.
 */
void vLIB_NAvgFilter_init(	xLIB_NAvgFilter_t* pxHandle,
							int32_t* piDataArr,
							uint32_t uiN	)
{
	/*	Create queue handle	*/
	pxHandle->xQueue = xQueueCreateStatic(	uiN,
	                                 	 	sizeof(int32_t),
											(uint8_t*)piDataArr,
											&pxHandle->xQueueStatic	);

	/*	Fill the queue with "uiN" zero samples	*/
	int32_t iZero = 0;
	for (uint32_t i = 0; i < uiN; i++)
	{
		xQueueSend(pxHandle->xQueue, (void*)&iZero, 0);
	}

	/*	Sum of samples so far is zero, and hence avg is also zero	*/
	pxHandle->iSum = 0;
	pxHandle->iAvg = 0;

	/*	Copy "uiN" to the handle	*/
	pxHandle->uiN = uiN;
}

/*
 * See header file for info.
 */
void vLIB_NAvgFilter_update(xLIB_NAvgFilter_t* pxHandle, int32_t iNewVal)
{
	/*	Read then remove front of the queue	*/
	int32_t iFront;
	xQueueReceive(pxHandle->xQueue, (void*)&iFront, 0);

	/*	Subtract front of the queue from the sum	*/
	pxHandle->iSum -= iFront;

	/*	Add new value to the sum and update the avg	*/
	pxHandle->iSum += iNewVal;
	pxHandle->iAvg = pxHandle->iSum / pxHandle->uiN;

	/*	Add new value to back of the queue	*/
	xQueueSend(pxHandle->xQueue, (void*)iNewVal, 0);
}

/*
 * See header file for info.
 */
void vLIB_NAvgFilter_updateFromISR(xLIB_NAvgFilter_t* pxHandle, int32_t iNewVal)
{
	/*	Read then remove front of the queue	*/
	int32_t iFront;
	xQueueReceiveFromISR(pxHandle->xQueue, (void*)&iFront, NULL);

	/*	Subtract front of the queue from the sum	*/
	pxHandle->iSum -= iFront;

	/*	Add new value to the sum and update the avg	*/
	pxHandle->iSum += iNewVal;
	pxHandle->iAvg = pxHandle->iSum / pxHandle->uiN;

	/*	Add new value to back of the queue	*/
	xQueueSendFromISR(pxHandle->xQueue, (void*)iNewVal, NULL);
}








