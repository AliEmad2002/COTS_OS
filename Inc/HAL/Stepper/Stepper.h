/*
 * Stepper.h
 *
 *  Created on: Jun 28, 2023
 *      Author: Ali Emad
 */

#ifndef HAL_OS_INC_STEPPER_STEPPER_H_
#define HAL_OS_INC_STEPPER_STEPPER_H_

#include "HAL/HardwareDelay/HardwareDelay.h"

/*******************************************************************************
 * API structures:
 ******************************************************************************/
typedef struct{
	/*			PRIVATE			*/
	int8_t cPosIncrementer;

	/*			PUBLIC			*/
	uint8_t ucStepPort;
	uint8_t ucStepPin;

	uint8_t ucDirPort;
	uint8_t ucDirPin;

	uint8_t ucEnPort;
	uint8_t ucEnPin;

	uint8_t ucIsEnabled; // (read only)

	int32_t iCurrentPos;

	/*	see description of "vHOS_Stepper_step()"	*/
	xHOS_HardwareDelay_t* pxHardwareDelayHandle;
}xHOS_Stepper_t;

/*******************************************************************************
 * API macros:
 ******************************************************************************/
#define ucHOS_STEPPER_DIR_FORWARD		0
#define ucHOS_STEPPER_DIR_BACKWARD		1

/*******************************************************************************
 * API functions:
 ******************************************************************************/

/*
 * Initializes stepper driver pins.
 */
void vHOS_Stepper_init(xHOS_Stepper_t* pxHandle);

/*
 * Set moving direction.
 *
 * Notes:
 * 		-	'ucDir' is one of: 'ucHOS_STEPPER_DIR_FORWARD' and 'ucHOS_STEPPER_DIR_BACKWARD'.
 *
 * 		-	This is an inline function.
 *
 * 		-	Before using an 'xHOS_Stepper_t' object, it is user's responsibility to
 * 			assure the object is not being used by any other task.
 */
void vHOS_Stepper_setDir(xHOS_Stepper_t* pxHandle, uint8_t ucDir);

/*
 * Moves the stepper one step forward immediately, in the currently set direction.
 * Note: This is an inline function.
 */
void vHOS_Stepper_stepSingle(xHOS_Stepper_t* pxHandle);

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
 *
 * 		-	This function is not ISR safe.
 */
void vHOS_Stepper_stepN(xHOS_Stepper_t* pxHandle, int32_t iN, int32_t iUsStepInterval);

/*
 * Enables stepper (power enable).
 */
void vHOS_Stepper_enable(xHOS_Stepper_t* pxHandle);

/*
 * Disables stepper (power disable).
 */
void vHOS_Stepper_disable(xHOS_Stepper_t* pxHandle);





#endif /* HAL_OS_INC_STEPPER_STEPPER_H_ */
