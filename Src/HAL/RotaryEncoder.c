/*
 * RotaryEncoder.c
 *
 *  Created on: Jun 29, 2023
 *      Author: Ali Emad
 */

/*	LIB	*/
#include <stdint.h>
#include <stdio.h>

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
#define vREAD_CHANNEL_A_FILTERED(pxHandle)                                                       \
{                                                                                                \
	/*	Update "prevFiltered" value	*/															 \
	(pxHandle)->ucAPrevLevelFiltered = (pxHandle)->ucALevelFiltered;					 		 \
	/*	Read real physical state	*/															 \
	(pxHandle)->ucANewLevel = ucPort_DIO_readPin((pxHandle)->ucAPort, (pxHandle)->ucAPin);       \
	/*	If new level is same as previous level (no noise occurred)	*/							 \
	if ((pxHandle)->ucANewLevel == (pxHandle)->ucAPrevLevel)                                     \
	{                                                                                            \
		(pxHandle)->ucNA++;                                                                      \
		/*	If N samples on the same level were measured	*/									 \
		if ((pxHandle)->ucNA == (pxHandle)->ucNFilter)                                           \
		{                                                                                        \
			/*	Channel's level is now stable and could be used	*/								 \
			(pxHandle)->ucALevelFiltered = (pxHandle)->ucANewLevel;                              \
			(pxHandle)->ucNA = 0;                                                                \
		}                                                                                        \
	}                                                                                            \
	/*	Otherwise, (noise occurred)	*/															 \
	else                                                                                         \
	{                                                                                            \
		(pxHandle)->ucNA = 0;                                                                    \
		(pxHandle)->ucAPrevLevel = (pxHandle)->ucANewLevel;                                      \
	}                                                                                            \
}

#define vREAD_CHANNEL_B_FILTERED(pxHandle)                                                       \
{                                                                                                \
	/*	Update "prevFiltered" value	*/															 \
	(pxHandle)->ucBPrevLevelFiltered = (pxHandle)->ucBLevelFiltered;					 		 \
	/*	Read real physical state	*/															 \
	(pxHandle)->ucBNewLevel = ucPort_DIO_readPin((pxHandle)->ucBPort, (pxHandle)->ucBPin);       \
	/*	If new level is same as previous level (no noise occurred)	*/							 \
	if ((pxHandle)->ucBNewLevel == (pxHandle)->ucBPrevLevel)                                     \
	{                                                                                            \
		(pxHandle)->ucNB++;                                                                      \
		/*	If N samples on the same level were measured	*/									 \
		if ((pxHandle)->ucNB == (pxHandle)->ucNFilter)                                           \
		{                                                                                        \
			/*	Channel's level is now stable and could be used	*/								 \
			(pxHandle)->ucBLevelFiltered = (pxHandle)->ucBNewLevel;                              \
			(pxHandle)->ucNB = 0;                                                                \
		}                                                                                        \
	}                                                                                            \
	/*	Otherwise, (noise occurred)	*/															 \
	else                                                                                         \
	{                                                                                            \
		(pxHandle)->ucNB = 0;                                                                    \
		(pxHandle)->ucBPrevLevel = (pxHandle)->ucBNewLevel;                                      \
	}                                                                                            \
}

#define ucIS_RISING_EDGE_ON_CHANNEL_A(pxHandle)	\
		((pxHandle)->ucALevelFiltered == 1 && (pxHandle)->ucAPrevLevelFiltered == 0)

/*******************************************************************************
 * Task function
 ******************************************************************************/
static void vTask(void* pvParams)
{
	xHOS_RotaryEncoder_t* pxHandle = (xHOS_RotaryEncoder_t*)pvParams;

	vTaskSuspend(pxHandle->xTask);

	TickType_t xLastWakeTime = xTaskGetTickCount();
	while(1)
	{
		/*	Read both channels, with applying the N-consicutive samples filtering method 	*/
		vREAD_CHANNEL_A_FILTERED(pxHandle);
		vREAD_CHANNEL_B_FILTERED(pxHandle);

		/*	if a rising edge occurred on channel A	*/
		if (ucIS_RISING_EDGE_ON_CHANNEL_A(pxHandle))
		{
			/*	if channel B is on high level	*/
			if (pxHandle->ucBLevelFiltered == 1)
				pxHandle->pfCWCallback(pxHandle->pvCWParams);

			/*	if channel B is on low level	*/
			else
				pxHandle->pfCCWCallback(pxHandle->pvCCWParams);
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
void vHOS_RotaryEncoder_init(xHOS_RotaryEncoder_t* pxHandle)
{
	/*	initialize A, B pins	*/
	vPort_DIO_initPinInput(pxHandle->ucAPort, pxHandle->ucAPin, 2);
	vPort_DIO_initPinInput(pxHandle->ucBPort, pxHandle->ucBPin, 2);

	/*	initialize handle's private variables	*/
	pxHandle->ucAPrevLevel = ucPort_DIO_readPin(pxHandle->ucAPort, pxHandle->ucAPin);
	pxHandle->ucBPrevLevel = ucPort_DIO_readPin(pxHandle->ucBPort, pxHandle->ucBPin);

	pxHandle->ucAPrevLevelFiltered = pxHandle->ucAPrevLevel;
	pxHandle->ucBPrevLevelFiltered = pxHandle->ucBPrevLevel;

	pxHandle->xLastActiveTimeStamp = 0;

	pxHandle->ucNA = 0;
	pxHandle->ucNB = 0;

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







