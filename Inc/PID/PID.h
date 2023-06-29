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
	float fE;
	float fI;
	float fD;

	StackType_t puxTaskStack[configMINIMAL_STACK_SIZE];
	StaticTask_t xTaskStatic;
	TaskHandle_t xTask;

	/*		PUBLIC 		*/
	/*
	 * Any of the following variables can be changed while handle is enabled and
	 * running, and effects will take place next update. They must be initialized
	 * with valid values before enabling the controller.
	 */
	uint32_t uiTimeIntervalMs;

	float fSetPoint;

	float fKi;
	float fKp;
	float fKd;

	float fIMax;
	float fIMin;

	float fDMax;
	float fDMin;

	float fOutMax;
	float fOutMin;

	float (*pfGetSample) (void);
	void (*pfUpdate) (float);
}xHOS_PID_t;

/*
 * Initializes task of a PID controller.
 *
 * Must be called before scheduler start.
 */
void vHOS_PID_init(xHOS_PID_t* pxHandle);

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
