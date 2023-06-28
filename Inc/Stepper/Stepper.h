/*
 * Stepper.h
 *
 *  Created on: Jun 28, 2023
 *      Author: Ali Emad
 */

#ifndef HAL_OS_INC_STEPPER_STEPPER_H_
#define HAL_OS_INC_STEPPER_STEPPER_H_

#include "Inc/HardwareDelay/HardwareDelay.h"

/*******************************************************************************
 * API structures:
 ******************************************************************************/
typedef struct{
	/*			PRIVATE			*/


	/*			PUBLIC			*/
	uint8_t ucStepPort;
	uint8_t ucStepPin;

	uint8_t ucDirPort;
	uint8_t ucDirPin;

	int32_t iCurrentPos;

	/*	see description of "vHOS_Stepper_step()"	*/
	xHOS_HardwareDelay_t* pxHardwareDelayHandle;
}xHOS_Stepper_t;

/*******************************************************************************
 * API functions:
 ******************************************************************************/

/*
 * Initializes stepper driver pins.
 */
void vHOS_Stepper_init(xHOS_Stepper_t* pxHandle);

/*
 * Moves the stepper one step forward, immediately.
 * Note: This is an inline function.
 */
void vHOS_Stepper_stepForward(xHOS_Stepper_t* pxHandle);

/*
 * Moves the stepper one step backward, immediately.
 * Note: This is an inline function.
 */
void vHOS_Stepper_stepBackward(xHOS_Stepper_t* pxHandle);

/*
 * Moves the stepper a number of steps.
 * 		-	'iN': Number of steps to move. Sign determines direction, positive is
 * 			forward and negative is backward.
 *
 * 		-	'iUsStepInterval': Time interval between each two consecutive steps in
 * 			microseconds.
 *
 * 		-	'pxHandle->pxHardwareDelayHandle' must be a valid and initialized handle,
 * 			its mutex would be taken on start of function and released on return.
 */
void vHOS_Stepper_step(xHOS_Stepper_t* pxHandle, int32_t iN, int32_t iUsStepInterval);



#endif /* HAL_OS_INC_STEPPER_STEPPER_H_ */
