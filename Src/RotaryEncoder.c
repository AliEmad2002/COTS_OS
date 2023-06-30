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
#include "Port/Port_DIO.h"

/*	HAL-OS	*/
#include "Inc/RTOS_PRI_Config.h"

/*	SELF	*/
#include "Inc/RotaryEncoder/RotaryEncoder.h"

/*******************************************************************************
 * Helping functions/macros:
 ******************************************************************************/

/*******************************************************************************
 * Task function
 ******************************************************************************/
static void vTask(void* pvParams)
{
	xHOS_RotaryEncoder_t* pxHandle = (xHOS_RotaryEncoder_t*)pvParams;

	uint8_t ucALevel;
	uint8_t ucBLevel;

	TickType_t xLastWakeTime = xTaskGetTickCount();
	while(1)
	{
		/*	Read both channels	*/
		ucALevel = ucPort_DIO_readPin(pxHandle->ucAPort, pxHandle->ucAPin);
		ucBLevel = ucPort_DIO_readPin(pxHandle->ucBPort, pxHandle->ucBPin);

		/*	if rotary is idle	*/
		if (xLastWakeTime - pxHandle->xLastActiveTimeStamp > pdMS_TO_TICKS(pxHandle->uiIdleTimeoutMs))
		{
			/*	else if an edge occurred on B first	*/
			if (	(ucBLevel == 1 && pxHandle->ucBPrevLevel == 0) ||
					(ucBLevel == 0 && pxHandle->ucBPrevLevel == 1)	)
			{
				pxHandle->xLastActiveTimeStamp = xLastWakeTime;
				pxHandle->cCurrentDirection = -1;
				pxHandle->ucFirstEdgeChannel = 1;
			}

			/*	else if an edge occurred on A first	*/
			else if (	(ucALevel == 1 && pxHandle->ucAPrevLevel == 0) ||
						(ucALevel == 0 && pxHandle->ucAPrevLevel == 1)	)
			{
				pxHandle->xLastActiveTimeStamp = xLastWakeTime;
				pxHandle->cCurrentDirection = 1;
				pxHandle->ucFirstEdgeChannel = 0;
			}
		}

		/*	else if it is not idle	*/
		/*	if a rising edge occurred on the "ucFirstEdgeChannel"	*/
		if (	(pxHandle->ucFirstEdgeChannel == 0 && ucALevel == 1 && pxHandle->ucAPrevLevel == 0)	||
				(pxHandle->ucFirstEdgeChannel == 1 && ucBLevel == 1 && pxHandle->ucBPrevLevel == 0)	)
		{
			pxHandle->xLastActiveTimeStamp = xLastWakeTime;
			pxHandle->iCount += pxHandle->cCurrentDirection;
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
	pxHandle->cCurrentDirection = 1;
	pxHandle->iCount = 0;
	pxHandle->ucFirstEdgeChannel = 0;

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

__attribute__((always_inline)) inline
int32_t iHOS_RotaryEncoder_getCount(xHOS_RotaryEncoder_t* pxHandle)
{
	return pxHandle->iCount;
}












