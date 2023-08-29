/*
 * UltraSonicDistanceSynchronizer.h
 *
 *  Created on: Aug 30, 2023
 *      Author: Ali Emad
 *
 * This driver is used to maintain accurate / acceptable results out of multiple
 * sensor handles.
 *
 * Having multiple sensor handles running at the same time, without using the
 * synchronizer, will possibly introduce the following problems:
 * 		-	In case of two or more sensors echo at the exact same time, they would
 * 			have their associated EXTI handlers deferred, resulting in non-accurate
 * 			timestamping.
 * 		-	In case of two or more sensors transmit ultra-sonic beams at the same
 * 			time, some of them may echo wrongly due to beams interference.
 */

#ifndef COTS_OS_INC_HAL_ULTRASONICDISTANCE_ULTRASONICDISTANCESYNCHRONIZER_H_
#define COTS_OS_INC_HAL_ULTRASONICDISTANCE_ULTRASONICDISTANCESYNCHRONIZER_H_

#include "HAL/UltraSonicDistance/UltraSonicDistance.h"

typedef struct{
	/*		PRIVATE		*/
	StaticTask_t xTaskStatic;
	TaskHandle_t xTask;
	StackType_t xTaskStack[configMINIMAL_STACK_SIZE];

	/*		PUBLIC		*/
	uint8_t ucNumberOfSensors;

	xHOS_UltraSonicDistance_t* pxSensorArr;

	uint8_t ucSamplesPerseconds;	// maximumly: 40-sample per second.
}xHOS_UltraSonicDistanceSynchronizer_t;


/*******************************************************************************
 * API functions:
 ******************************************************************************/
/*
 * Initializes handle.
 *
 * Notes:
 * 		-	All public parameters of the handle must be initialized with valid values.
 *
 * 		-	Must be called before scheduler start.
 */
void vHOS_UltraSonicDistanceSynchronizer_init(
	xHOS_UltraSonicDistanceSynchronizer_t* pxHandle	);



#endif /* COTS_OS_INC_HAL_ULTRASONICDISTANCE_ULTRASONICDISTANCESYNCHRONIZER_H_ */
