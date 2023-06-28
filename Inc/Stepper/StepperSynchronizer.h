/*
 * StepperSynchronizer.h
 *
 *  Created on: Jun 28, 2023
 *      Author: Ali Emad
 */

#ifndef HAL_OS_INC_STEPPER_STEPPERSYNCHRONIZER_H_
#define HAL_OS_INC_STEPPER_STEPPERSYNCHRONIZER_H_

#include "Inc/Stepper/Stepper.h"

typedef struct{
	/*			PRIVATE			*/


	/*			PUBLIC			*/
	xHOS_Stepper_t* pxStepperArr;

	uint8_t ucNumberOfSteppers;

	xHOS_HardwareDelay_t* pxHardwareDelayHandle;
}xHOS_StepperSynchronizer_t;

/*
 * Notes:
 * 		-	'piNArr' is an array of number of steps to be made by each stepper.
 * 			Such that 'pxHandle->pxStepperArr[i]' moves by 'piNArr[i]'.
 *
 * 		-	'uiSpeedMagnitudeInverseUs' is inverse of total speed in (microsecond/step).
 * 			Such that total speed = sqrt( (stepper0 speed) ^ 2 + (stepper1 speed) ^ 2 + ... )
 */
void vHOS_StepperSynchronizer_move(	xHOS_StepperSynchronizer_t* pxHandle,
									int32_t* piNArr,
									uint32_t uiSpeedMagnitudeInverseUs);


#endif /* HAL_OS_INC_STEPPER_STEPPERSYNCHRONIZER_H_ */
