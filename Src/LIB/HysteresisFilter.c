/*
 * HysteresisFilter.c
 *
 *  Created on: Jan 5, 2024
 *      Author: Ali Emad
 */

/*	LIB	*/
#include <stdint.h>

/*	SELF	*/
#include "LIB/HysteresisFilter/HysteresisFilter.h"


void vLIB_HysteresisFilter_init(xLIB_HysteresisFilter_t* pxHandle)
{
	pxHandle->ucCurrentOutputState = 0;
}

void vLIB_HysteresisFilter_update(	xLIB_HysteresisFilter_t* pxHandle,
									int32_t iNewSample	)
{
	/*	If new sample is less that lowThreshold, output state is 1.	*/
	if (iNewSample < pxHandle->iLowThreshold)
		pxHandle->ucCurrentOutputState = 1;

	/*	Otherwise, if new sample is larger that highThreshold, output state is 0.	*/
	else if (iNewSample > pxHandle->iHighThreshold)
		pxHandle->ucCurrentOutputState = 0;

	/*	Otherwise, if new sample is in between, output state is not changed	*/
}
