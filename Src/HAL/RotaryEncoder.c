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


/*******************************************************************************
 * Task function
 ******************************************************************************/
static void vTask(void* pvParams)
{
	xHOS_RotaryEncoder_t* pxHandle = (xHOS_RotaryEncoder_t*)pvParams;

	uint8_t ucANewLevel;
	uint8_t ucBNewLevel;

	uint8_t ucAPrevLevel = 0;
	uint8_t ucBPrevLevel = 0;

	uint8_t ucALevel;
	uint8_t ucBLevel;

	uint8_t ucNA = 0;
	uint8_t ucNB = 0;

	vTaskSuspend(pxHandle->xTask);

	TickType_t xLastWakeTime = xTaskGetTickCount();
	while(1)
	{
		/*	Read both channels, with applying the N-consicutive samples filtering method 	*/
		ucANewLevel = ucPort_DIO_readPin(pxHandle->ucAPort, pxHandle->ucAPin);
		if (ucANewLevel == ucAPrevLevel)
		{
			ucNA++;
			if (ucNA == pxHandle->ucNFilter)
			{
				ucALevel = ucANewLevel;
				ucNA = 0;
			}
		}
		else
		{
			ucNA = 0;
		}
		ucAPrevLevel = ucANewLevel;

		ucBNewLevel = ucPort_DIO_readPin(pxHandle->ucBPort, pxHandle->ucBPin);
		if (ucBNewLevel == ucBPrevLevel)
		{
			ucNB++;
			if (ucNB == pxHandle->ucNFilter)
			{
				ucBLevel = ucBNewLevel;
				ucNB = 0;
			}
		}
		else
		{
			ucNB = 0;
		}
		ucBPrevLevel = ucBNewLevel;

		/*	if a rising edge occurred on channel A	*/
		if (ucALevel == 1 && pxHandle->ucAPrevLevel == 0)
		{
			/*	if channel B is on high level	*/
			if (ucBLevel == 1)
				pxHandle->pfCWCallback(pxHandle->pvCWParams);

			/*	if channel B is on low level	*/
			else
				pxHandle->pfCCWCallback(pxHandle->pvCCWParams);
		}

		/*	Update levels	*/
		pxHandle->ucAPrevLevel = ucALevel;
		pxHandle->ucBPrevLevel = ucBLevel;

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

	pxHandle->xLastActiveTimeStamp = 0;

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







