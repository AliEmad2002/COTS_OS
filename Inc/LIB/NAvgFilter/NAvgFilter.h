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

typedef struct{
	/*		PRIVATE		*/
	StaticQueue_t xQueueStatic;
	QueueHandle_t xQueue;

	int32_t iSum;

	/*		PUBLIC		*/
	uint32_t uiN;
	int32_t iAvg;	// (Read-only).
}xLIB_NAvgFilter_t;


/*
 * Initializes handle.
 *
 * 		-	"piDataArr": Array of data. Must be allocated by user, casted, and
 *	 		assigned to this pointer.
 *
 *	 	-	"uiN": Number of samples in the averaging window. ("piDataArr" must
 *	 		be of a length that is greater than or equal to "uiN").
 *
 *	 	-	"uiElemSizeInWords": Size of single element of the data type used
 *	 		(in words). Minimum value is 1.
 */
void vLIB_NAvgFilter_init(	xLIB_NAvgFilter_t* pxHandle,
							int32_t* piDataArr,
							uint32_t uiN	);

/*
 * Updates filter with new value.
 *
 * 		-	Must not be used in an ISR.
 */
void vLIB_NAvgFilter_update(xLIB_NAvgFilter_t* pxHandle, int32_t iNewVal);

/*
 * Updates filter with new value.
 *
 * 		-	Could be used in an ISR.
 */
void vLIB_NAvgFilter_updateFromISR(xLIB_NAvgFilter_t* pxHandle, int32_t iNewVal);















#endif /* COTS_OS_INC_LIB_NAVGFILTER_NAVGFILTER_H_ */
