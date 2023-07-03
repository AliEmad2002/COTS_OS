/*
 * StepperSynchronizer.c
 *
 *  Created on: Jun 28, 2023
 *      Author: Ali Emad
 */

/*	LIB	*/
#include <stdint.h>
#include <math.h>

/*	FreeRTOS	*/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/*	SELF	*/
#include "HAL/Stepper/StepperSynchronizer.h"

/*******************************************************************************
 * Helping functions:
 ******************************************************************************/
static inline uint8_t ucGetMaxIndex(int32_t* piArr, int8_t ucN)
{
	uint8_t ucMaxIndex = 0;

	for (uint8_t i = 1; i < ucN; i++)
	{
		if (piArr[i] > piArr[ucMaxIndex])
			ucMaxIndex = i;
	}

	return ucMaxIndex;
}

static inline uint32_t uiGetNorm(int32_t* piArr, int8_t ucN)
{
	uint64_t ulNormSquared = 0;

	for (uint8_t i = 0; i < ucN; i++)
	{
		ulNormSquared += piArr[i] * piArr[i];
	}

	uint32_t uiNorm = sqrt(ulNormSquared);
	return uiNorm;
}

/*******************************************************************************
 * API functions:
 ******************************************************************************/
/*
 * See header file for info.
 */
void vHOS_StepperSynchronizer_move(	xHOS_StepperSynchronizer_t* pxHandle,
									int32_t* piNArr,
									uint32_t uiSpeedMagnitudeInverseUs	)
{
	/*	Extract handle parameters	*/
	uint8_t ucN = pxHandle->ucNumberOfSteppers;
	xHOS_Stepper_t* pxStepperArr = pxHandle->pxStepperArr;
	uint32_t* puiStepsDoneArr = pxHandle->puiStepsDoneArr;

	/*
	 * Set directions of all steppers, and make 'piNArr' a magnitude array.
	 * Also, reset the 'puiStepsDoneArr[]' array	*/
	for (uint8_t i = 0; i < ucN; i++)
	{
		if (piNArr[i] >= 0)
			vHOS_Stepper_setDir(&pxHandle->pxStepperArr[i], ucHOS_STEPPER_DIR_FORWARD);
		else
		{
			vHOS_Stepper_setDir(&pxHandle->pxStepperArr[i], ucHOS_STEPPER_DIR_BACKWARD);
			piNArr[i] = -piNArr[i];
		}

		puiStepsDoneArr[i] = 0;
	}

	/*	Get necessary parameters	*/
	uint8_t ucMaxIndex = ucGetMaxIndex(piNArr, ucN);

	uint32_t uiNorm = uiGetNorm(piNArr, ucN);

	uint32_t uiStepIntervalUs = ( (uint64_t)uiSpeedMagnitudeInverseUs *
									 (uint64_t)piNArr[ucMaxIndex] ) / uiNorm;

	uint32_t uiStepIntervalTicks = ( (uint64_t)uiStepIntervalUs *
									 (uint64_t)pxHandle->pxHardwareDelayHandle->uiTicksPerSecond ) /
									 1000000ul;

	/*	Take 'HardwareDelay_t' unit mutex	*/
	xSemaphoreTake(pxHandle->pxHardwareDelayHandle->xMutex, portMAX_DELAY);

	/*	Move steppers synchronously	*/
	uint32_t uiTheo;
	while(puiStepsDoneArr[ucMaxIndex] < piNArr[ucMaxIndex])
	{
		/*	Step stepper[maxIndex]	*/
		vHOS_Stepper_stepSingle(&pxStepperArr[ucMaxIndex]);
		puiStepsDoneArr[ucMaxIndex]++;

		/*	Loop on all other steppers	*/
		for (uint8_t i = 0; i < ucN; i++)
		{
			if (i == ucMaxIndex)
				continue;

			/*	check if it's time to step	*/
			uiTheo = ((uint64_t)puiStepsDoneArr[ucMaxIndex] * (uint64_t)piNArr[i]) / piNArr[ucMaxIndex];
			if (puiStepsDoneArr[i] < uiTheo)
			{
				vHOS_Stepper_stepSingle(&pxStepperArr[i]);
				puiStepsDoneArr[i]++;
			}
		}

		/*	HW (RTOS handled) Delay until next step of stepper[maxIndex]	*/
		vHOS_HardwareDelay_delayUs(pxHandle->pxHardwareDelayHandle, uiStepIntervalTicks);
	}

	/*	Give 'HardwareDelay_t' unit mutex	*/
	xSemaphoreGive(pxHandle->pxHardwareDelayHandle->xMutex);
}

















