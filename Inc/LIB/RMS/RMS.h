/*
 * RMS.h
 *
 *  Created on: Jun 15, 2024
 *      Author: Ali Emad
 */

#ifndef COTS_OS_INC_LIB_RMS_RMS_H_
#define COTS_OS_INC_LIB_RMS_RMS_H_

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

typedef struct{
	/*		PRIVATE		*/
	StaticQueue_t xQueueStatic;
	QueueHandle_t xQueue;

	StaticSemaphore_t xMutexStatic;
	SemaphoreHandle_t xMutex;

	float fSum;

	float fX0;
	float fXN_1;

	uint32_t uiN;
}xLIB_RMS_t;


/*
 * Initializes handle.
 *
 * 		-	"piDataArr": Array of data, used as queue's memory. Must be allocated
 * 			by user, casted, and assigned to this pointer.
 *
 *	 	-	"uiN": Number of samples in the averaging window. ("piDataArr" must
 *	 		be of a length that is greater than or equal to: ("uiN" - 2)).
 */
void vLIB_RMS_init(	xLIB_RMS_t* pxHandle,
					float* piDataArr,
					uint32_t uiN	);

/*
 * Locks handle.
 */
uint8_t ucLIB_RMS_lock(xLIB_RMS_t* pxHandle, TickType_t xTimeout);

/*
 * Unlocks handle.
 */
vLIB_RMS_unlock(xLIB_RMS_t* pxHandle);

/*
 * Updates filter with new value.
 *
 * 		-	Must not be used in an ISR.
 *
 * 		-	Mutex of the handle must be locked first before updating.
 */
void vLIB_RMS_update(xLIB_RMS_t* pxHandle, float fNewVal);

/*
 * Updates filter with new value.
 *
 * 		-	Could be used in an ISR.
 *
 * 		-	Mutex of the handle must be locked first before updating.
 */
void vLIB_RMS_updateFromISR(xLIB_RMS_t* pxHandle, float fNewVal);

float fLIB_RMS_getValue(xLIB_RMS_t* pxHandle);






#endif /* COTS_OS_INC_LIB_RMS_RMS_H_ */
