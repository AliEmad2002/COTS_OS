/*
 * Vector.h
 *
 *  Created on: Sep 23, 2023
 *      Author: Ali Emad
 */

#ifndef VECTOR_H_
#define VECTOR_H_

#include "LIB/Vector/Vector_Config.h"

#include "FreeRTOS.h"
#include "semphr.h"

typedef struct{
	/*		PRIVATE		*/
	xCONF_VECTOR_ELEM_TYPE pxArr[xCONF_VECTOR_MAX_SIZE];
	uint32_t uiSize;

	SemaphoreHandle_t xMutex;
	StaticSemaphore_t xMutexStatic;
}xLIB_Vector_t;

/*
 * Initializes vector handle.
 */
void vLIB_Vector_init(xLIB_Vector_t* pxVector);

/*
 * Mutex locking and unlocking of a vector handle.
 *
 * Notes:
 * 		-	Vector handle should not be used without being locked first.
 *
 * 		-	Vector handle can only be locked and unlocked form inside a task, it
 * 			is not possible from ISR.
 */
#define ucLIB_VECTOR_LOCK(pxVector, xTimeout)	\
	(	xSemaphoreTake((pxVector)->xMutex, (xTimeout))	)

#define vLIB_VECTOR_RELEASE(pxVector)	\
	(	xSemaphoreGive((pxVector)->xMutex)	)

/*
 *	Gets number of elements currently available in the given vector handle.
 */
#define uiLIB_VECTOR_GET_SIZE(pxVector)		(	(const uint32_t)((pxVector)->uiSize)	)

/*
 * Adds a new element to the end of the vector handle.
 *
 * Notes:
 * 		-	Vector overflow is not checked within this macro. It may be checked
 * 			before adding new element (by comparing it to  "xCONF_VECTOR_MAX_SIZE").
 */
#define vLIB_VECTOR_ADD_BACK(pxVector, xNewElem)	\
	(	(pxVector)->pxArr[(pxVector)->uiSize++] = (xNewElem)	)

/*
 * Removes last element in the vector.
 *
 * Notes:
 * 		-	Vector overflow is not checked within this macro. It may be checked
 * 			before adding new element (by comparing it to  zero).
 */
#define vLIB_VECTOR_REMOVE_BACK(pxVector)	(	(pxVector)->uiSize--	)

/*
 * Gets i-th element in the vector.
 *
 * Notes:
 * 		-	This macro is read / write (For example: it could be on either side of a '=' operator).
 *
 * 		-	Index 'i' is not checked within this macro.
 */
#define xLIB_VECTOR_GET_ELEM(pxVector, i)	(	(pxVector)->pxArr[(i)]	)

/*
 * Swaps two elements in a vector.
 *
 * Notes:
 * 		-	Indexes 'i' & 'j' are not checked within this macro.
 */
#define vLIB_VECTOR_SWAP(pxVector, i, j)									\
{                                                                           \
	xCONF_VECTOR_ELEM_TYPE _xTemp = xLIB_VECTOR_GET_ELEM(pxVector, i);       \
	xLIB_VECTOR_GET_ELEM(pxVector, i) = xLIB_VECTOR_GET_ELEM(pxVector, j);  \
	xLIB_VECTOR_GET_ELEM(pxVector, j) = _xTemp;                              \
}




#endif /* VECTOR_H_ */
