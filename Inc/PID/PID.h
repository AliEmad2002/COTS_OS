/*
 * PID.h
 *
 *  Created on: Jun 28, 2023
 *      Author: Ali Emad
 */

#ifndef HAL_OS_INC_PID_PID_H_
#define HAL_OS_INC_PID_PID_H_

#include "FreeRTOS.h"

typedef struct{
	/*		PRIVATE		*/
	float pfCArr[3];

	uint32_t uiTimeIntervalMs;

	float (*pfGetSample) (void);
	void (*pfUpdate) (float);

	float pfXPrev[2];
	float pfYPrev[2];

	float fSetPoint;

	StackType_t puxTaskStack[configMINIMAL_STACK_SIZE];
	StaticTask_t xTaskStatic;
	TaskHandle_t xTask;
	void* pvParam;
}xHOS_PID_t;

/*
 * Initializes task of a PID controller.
 *
 * Must be called before scheduler start.
 */
void vHOS_PID_init(	xHOS_PID_t* pxHandle,
					float fKd, float fKp, float fKi,
					uint32_t uiTimeIntervalMs,
					float fSetPointInitial,
					float (*pfGetSample) (void),
					void (*pfUpdate) (float)	);

/*
 * Sets set-point of the controller.
 * This function is inline.
 */
void vHOS_PID_writeSetPoint(xHOS_PID_t* pxHandle, float fSetPoint);

/*
 * Enables PID controller.
 * Notice that PID controller is disabled on initialization by default.
 * This function is inline.
 */
void vHOS_PID_enable(xHOS_PID_t* pxHandle);

/*
 * Disables PID controller.
 * Notice that PID controller is disabled on initialization by default.
 * This function is inline.
 */
void vHOS_PID_disable(xHOS_PID_t* pxHandle);

#endif /* HAL_OS_INC_PID_PID_H_ */
