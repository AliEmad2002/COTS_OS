/*
 * NAvgFilter.h
 *
 *  Created on: Aug 9, 2023
 *      Author: Ali Emad
 */

#ifndef COTS_OS_INC_LIB_NAVGFILTER_NAVGFILTER_H_
#define COTS_OS_INC_LIB_NAVGFILTER_NAVGFILTER_H_

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

typedef struct{
	/*		PRIVATE		*/
	StaticQueue_t xQueueStatic;
	QueueHandle_t xQueue;

	int32_t iSum;

	/*		PUBLIC		*/
	/*
	 * Number of samples of a single filtering window.
	 */
	uint32_t uiN;

	/*
	 * Resultant average value.
	 *
	 * Notes:
	 * 		-	Read only.
	 * 		-	Mutex of the handle must be locked first before reading.
	 */
	int32_t iAvg;

	StaticSemaphore_t xMutexStatic;
	SemaphoreHandle_t xMutex;
}xLIB_NAvgFilter_t;


/*
 * Initializes handle.
 *
 * 		-	"piDataArr": Array of data, used as queue's memory. Must be allocated
 * 			by user, casted, and assigned to this pointer.
 *
 *	 	-	"uiN": Number of samples in the averaging window. ("piDataArr" must
 *	 		be of a length that is greater than or equal to "uiN").
 *
 *	 	-	Must be called before scheduler start.
 */
void vLIB_NAvgFilter_init(	xLIB_NAvgFilter_t* pxHandle,
							int32_t* piDataArr,
							uint32_t uiN	);

/*
 * Updates filter with new value.
 *
 * 		-	Must not be used in an ISR.
 *
 * 		-	Mutex of the handle must be locked first before updating.
 */
void vLIB_NAvgFilter_update(xLIB_NAvgFilter_t* pxHandle, int32_t iNewVal);

/*
 * Updates filter with new value.
 *
 * 		-	Could be used in an ISR.
 *
 * 		-	Mutex of the handle must be locked first before updating.
 */
void vLIB_NAvgFilter_updateFromISR(xLIB_NAvgFilter_t* pxHandle, int32_t iNewVal);















#endif /* COTS_OS_INC_LIB_NAVGFILTER_NAVGFILTER_H_ */
