/*
 * UltraSonicDistance.h
 *
 *  Created on: Aug 29, 2023
 *      Author: Ali Emad
 *
 *
 * This driver is used when having one sensor, or independent multiple sensors.
 * Otherwise, if having multiple dependent sensors (dependent here means running at
 * the same sampling rate), it would be more accurate to use
 * "UltraSonicDistanceSynchronizer.h" driver alongside this driver.
 *
 * Notes:
 * 		-	In order for this driver to operate, HWTime must be initialized
 * 			@ at least F_TIM = 500KHz
 */

#ifndef COTS_OS_INC_HAL_ULTRASONICDISTANCE_ULTRASONICDISTANCE_H_
#define COTS_OS_INC_HAL_ULTRASONICDISTANCE_ULTRASONICDISTANCE_H_

#include "LIB/NAvgFilter/NAvgFilter.h"

/*******************************************************************************
 * Structures:
 ******************************************************************************/
typedef struct{
	/*		PRIVATE		*/
	StaticTask_t xTaskStatic;
	TaskHandle_t xTask;
	StackType_t xTaskStack[configMINIMAL_STACK_SIZE];

	StaticSemaphore_t xNewReadSemaphoreStatic;
	SemaphoreHandle_t xNewReadSemaphore;

	StaticSemaphore_t xSyncSemaphoreStatic;
	SemaphoreHandle_t xSyncSemaphore;

	uint64_t risingEchoTimestamp;
	uint64_t fallingEchoTimestamp;

	xLIB_NAvgFilter_t xFilter;

	/*		PUBLIC		*/
	uint8_t ucTrigPort;
	uint8_t ucTrigPin;

	uint8_t ucEchoPort;	// must be a pin that
	uint8_t ucEchoPin;	// has EXTI capability.

	uint8_t ucSamplesPerseconds;	// maximumly: 40-sample per second.

	uint8_t ucIsConnectedToSynchronizer;

	/*
	 * If the read sample is higher than this value, driver ignores it.
	 */
	int32_t iSaturationDistance;
}xHOS_UltraSonicDistance_t;


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
void vHOS_UltraSonicDistance_init(	xHOS_UltraSonicDistance_t* pxHandle,
									uint32_t uiNFilter,
									int32_t piFilterMemory[uiNFilter]	);

/*
 * Returns distance (filtered) in mm.
 */
uint32_t uiHOS_UltraSonicDistance_getDist(xHOS_UltraSonicDistance_t* pxHandle);

















#endif /* COTS_OS_INC_HAL_ULTRASONICDISTANCE_ULTRASONICDISTANCE_H_ */
