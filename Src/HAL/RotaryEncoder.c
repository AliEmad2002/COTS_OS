/*
 * RotaryEncoder.c
 *
 *  Created on: Jun 29, 2023
 *      Author: Ali Emad
 */

/*	LIB	*/
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include "LIB/BinaryFilter/BinaryFilter.h"

/*	FreeRTOS	*/
#include "FreeRTOS.h"
#include "task.h"

/*	MCAL (Ported)	*/
#include "MCAL_Port/Port_DIO.h"

/*	HAL-OS	*/
#include "RTOS_PRI_Config.h"

/*	SELF	*/
#include "HAL/RotaryEncoder/RotaryEncoder.h"

/*******************************************************************************
 * Helping functions/macros:
 ******************************************************************************/
#define ucIS_FALLING_EDGE_ON_CHANNEL_A(pxHandle)	\
		(	(pxHandle)->xAFilter.ucLevelFiltered == 0 && \
			(pxHandle)->xAFilter.ucPrevLevelFiltered == 1	)

/*******************************************************************************
 * Task function
 ******************************************************************************/
static void vTask(void* pvParams)
{
	xHOS_RotaryEncoder_t* pxHandle = (xHOS_RotaryEncoder_t*)pvParams;

	uint8_t ucANewLevel, ucBNewLevel;
	int32_t iPosAtPrevSpeedUpdate = 0;
	TickType_t xTimeAtPrevSpeedUpdate = 0;
	TickType_t xDeltaT;
	int32_t iDeltaPos;
	int32_t iPrevDeltaPos = 0;
	TickType_t xFirstZeroPosChangeTime = 0; // time of the sample at which position
											// stopped changing.


	vTaskSuspend(pxHandle->xTask);

	TickType_t xLastWakeTime = xTaskGetTickCount();
	while(1)
	{
		/*	Read both channels, with applying the N-consicutive samples filtering method 	*/
		ucANewLevel = ucPort_DIO_readPin(pxHandle->ucAPort, pxHandle->ucAPin);
		ucBNewLevel = ucPort_DIO_readPin(pxHandle->ucBPort, pxHandle->ucBPin);

		vLIB_BinaryFilter_updateFilter(&pxHandle->xAFilter, ucANewLevel);
		vLIB_BinaryFilter_updateFilter(&pxHandle->xBFilter, ucBNewLevel);

		/*	if a falling edge occurred on channel A	*/
		if (ucIS_FALLING_EDGE_ON_CHANNEL_A(pxHandle))
		{
			/*	if channel B is on high level	*/
			if (pxHandle->xBFilter.ucLevelFiltered == 1)
			{
				pxHandle->iPos++;
				if (pxHandle->ucEnableCWCallback)
					pxHandle->pfCWCallback(pxHandle->pvCWParams);
			}

			/*	if channel B is on low level	*/
			else
			{
				pxHandle->iPos--;
				if (pxHandle->ucEnableCCWCallback)
					pxHandle->pfCCWCallback(pxHandle->pvCCWParams);
			}
		}

		/*	speed update	*/
		if (pxHandle->ucEnableSpeedUpdate)
		{
			/*	If speed update period passed	*/
			xDeltaT = xLastWakeTime - xTimeAtPrevSpeedUpdate;
			if (xDeltaT >= pdMS_TO_TICKS(pxHandle->uiSpeedUpdatePeriodMs))
			{
				iDeltaPos = pxHandle->iPos - iPosAtPrevSpeedUpdate;

				/*	If position has not changed	*/
				if (iDeltaPos == 0)
				{
					/*	If this is the first time in a row	*/
					if (iPrevDeltaPos != 0)
					{
						/*	Take timestamp for when position started to stop changing.	*/
						xFirstZeroPosChangeTime = xLastWakeTime;
					}

					/*
					 * Otherwise, check if speed update dead time has passed,
					 * speed is zero.
					 */
					else
					{
						if (xLastWakeTime - xFirstZeroPosChangeTime >= pxHandle->uiSpeedDeadTimeMs)
						{
							pxHandle->iSpeed = 0;
							xTimeAtPrevSpeedUpdate = xLastWakeTime;
						}
					}
				}

				/*	Otherwise, if position changes, update speed normally	*/
				else
				{
					pxHandle->iSpeed = ((iDeltaPos * 1000 * 60) / (int32_t)xDeltaT);
					xTimeAtPrevSpeedUpdate = xLastWakeTime;
				}

				iPosAtPrevSpeedUpdate = pxHandle->iPos;
				iPrevDeltaPos = iDeltaPos;
			}
		}

		/*	Task is blocked until next sample time	*/
		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(pxHandle->uiSamplePeriodMs));
	}
}

/*******************************************************************************
 * API functions
 ******************************************************************************/
/*
 * See header file for info.
 */
void vHOS_RotaryEncoder_init(xHOS_RotaryEncoder_t* pxHandle, uint8_t ucNFilter)
{
	/*	initialize A, B pins	*/
	vPort_DIO_initPinInput(pxHandle->ucAPort, pxHandle->ucAPin, 2);
	vPort_DIO_initPinInput(pxHandle->ucBPort, pxHandle->ucBPin, 2);

	/*	initialize handle's variables	*/
	pxHandle->xAFilter.ucNFilter = ucNFilter;
	vLIB_BinaryFilter_init(&pxHandle->xAFilter);

	pxHandle->xBFilter.ucNFilter = ucNFilter;
	vLIB_BinaryFilter_init(&pxHandle->xBFilter);

	pxHandle->iPos = 0;

	/*	create task	*/
	static uint8_t ucCreatedObjectsCount = 0;
	char pcTaskName[configMAX_TASK_NAME_LEN];
	sprintf(pcTaskName, "Rotary%d", ucCreatedObjectsCount++);

	pxHandle->xTask = xTaskCreateStatic(	vTask,
											pcTaskName,
											configMINIMAL_STACK_SIZE,
											(void*)pxHandle,
											configHOS_HARD_REAL_TIME_TASK_PRI,
											pxHandle->puxTaskStack,
											&pxHandle->xTaskStatic	);
}

/*
 * See header file for info.
 */
__attribute__((always_inline)) inline
void vHOS_RotaryEncoder_enable(xHOS_RotaryEncoder_t* pxHandle)
{
	vTaskResume(pxHandle->xTask);
}

/*
 * See header file for info.
 */
__attribute__((always_inline)) inline
void vHOS_RotaryEncoder_disable(xHOS_RotaryEncoder_t* pxHandle)
{
	vTaskSuspend(pxHandle->xTask);
}







