/*
 * UltraSonicDistanceSynchronizer.c
 *
 *  Created on: Aug 30, 2023
 *      Author: Ali Emad
 */

/*	LIB	*/
#include <stdint.h>
#include <stdio.h>

/*	FreeRTOS	*/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/*	MCAL (Ported)	*/

/*	HAL-OS	*/
#include "RTOS_PRI_Config.h"

/*	SELF	*/
#include "HAL/UltraSonicDistance/UltraSonicDistanceSynchronizer.h"


/*******************************************************************************
 * RTOS Task code:
 ******************************************************************************/
static void vTask(void* pvParams)
{
	xHOS_UltraSonicDistanceSynchronizer_t* pxHandle =
		(xHOS_UltraSonicDistanceSynchronizer_t*)pvParams;

	TickType_t xLastWakeTime = xTaskGetTickCount();
	while(1)
	{
		for (uint8_t i = 0; i < pxHandle->ucNumberOfSensors; i++)
		{
			/*	Trigger sensor's task to start a new sample	*/
			xSemaphoreGive(pxHandle->pxSensorArr[i].xSyncSemaphore);

			/*	Delay for sample_period / number_of_sensors	*/
			vTaskDelayUntil(
				&xLastWakeTime,
				pdMS_TO_TICKS(	1000ul / pxHandle->ucSamplesPerseconds /
								pxHandle->ucNumberOfSensors	)	);
		}
	}
}

/*******************************************************************************
 * API functions:
 ******************************************************************************/
/*
 * See header file for info.
 */
void vHOS_UltraSonicDistanceSynchronizer_init(
	xHOS_UltraSonicDistanceSynchronizer_t* pxHandle	)
{
	/*	Initialize task	*/
	static uint8_t ucCreatedObjectsCount = 0;
	char pcTaskName[configMAX_TASK_NAME_LEN];
	sprintf(pcTaskName, "USDS%d", ucCreatedObjectsCount++);

	pxHandle->xTask = xTaskCreateStatic(	vTask,
											pcTaskName,
											configMINIMAL_STACK_SIZE,
											(void*)pxHandle,
											configHOS_MID_REAL_TIME_TASK_PRI,
											pxHandle->xTaskStack,
											&pxHandle->xTaskStatic	);
}
