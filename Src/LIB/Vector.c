/*
 * Vector.c
 *
 *  Created on: Sep 23, 2023
 *      Author: Ali Emad
 */


#include "stdint.h"

#include "LIB/Vector/Vector.h"

void vLIB_Vector_init(xLIB_Vector_t* pxVector)
{
	pxVector->uiSize = 0;

	pxVector->xMutex = xSemaphoreCreateMutexStatic(&pxVector->xMutexStatic);
	xSemaphoreGive(pxVector->xMutex);
}
