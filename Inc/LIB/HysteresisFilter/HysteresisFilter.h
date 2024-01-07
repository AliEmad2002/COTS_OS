/*
 * HysteresisFilter.h
 *
 *  Created on: Jan 5, 2024
 *      Author: Ali Emad
 */

#ifndef COTS_OS_INC_LIB_HYSTERESISFILTER_HYSTERESISFILTER_H_
#define COTS_OS_INC_LIB_HYSTERESISFILTER_HYSTERESISFILTER_H_

typedef struct{
	int32_t iLowThreshold;
	int32_t iHighThreshold;
	uint8_t ucCurrentOutputState;
}xLIB_HysteresisFilter_t;

void vLIB_HysteresisFilter_init(xLIB_HysteresisFilter_t* pxHandle);

void vLIB_HysteresisFilter_update(	xLIB_HysteresisFilter_t* pxHandle,
									int32_t iNewSample	);

#endif /* COTS_OS_INC_LIB_HYSTERESISFILTER_HYSTERESISFILTER_H_ */
