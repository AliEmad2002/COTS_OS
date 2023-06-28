/*
 * PID.c
 *
 *  Created on: Jun 28, 2023
 *      Author: Ali Emad
 */

/*	LIB	*/
#include <stdint.h>

/*	FreeRTOS	*/
#include "FreeRTOS.h"
#include "task.h"

/*	HAL-OS	*/
#include "Inc/RTOS_PRI_Config.h"

/*	SELF	*/
#include "Inc/PID/PID.h"

/*******************************************************************************
 * Task function
 ******************************************************************************/
static void vTask(void* pvParams)
{
	xHOS_PID_t* pxHandle = (xHOS_PID_t*)pvParams;
	vTaskSuspend(pxHandle->xTask);

	TickType_t xLastWakeTime = xTaskGetTickCount();
	float fX, fY;
	while(1)
	{
		fX = pxHandle->fSetPoint - pxHandle->pfGetSample();

		fY =	fX						*	pxHandle->pfCArr[0] +
				pxHandle->pfXPrev[0]	*	pxHandle->pfCArr[1] +
				pxHandle->pfXPrev[1]	*	pxHandle->pfCArr[2] +
				pxHandle->pfYPrev[1];

		pxHandle->pfUpdate(fY);

		pxHandle->pfXPrev[1] = pxHandle->pfXPrev[0];
		pxHandle->pfXPrev[0] = fX;

		pxHandle->pfYPrev[1] = pxHandle->pfYPrev[0];
		pxHandle->pfYPrev[0] = fY;

		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(pxHandle->uiTimeIntervalMs));
	}
}

/*******************************************************************************
 * API functions
 ******************************************************************************/
/*
 * See header file for info.
 */
void vHOS_PID_init(	xHOS_PID_t* pxHandle,
					float fKd, float fKp, float fKi,
					uint32_t uiTimeIntervalMs,
					float fSetPointInitial,
					float (*pfGetSample) (void),
					void (*pfUpdate) (float)	)
{
	/*	Initialize constants array	*/
	float fT = (float)uiTimeIntervalMs / 1000.0f;

	pxHandle->pfCArr[0] = 2.0f * fT * fKp + fT * fT * fKi + 4.0f * fKd;
	pxHandle->pfCArr[1] = 2.0f * fT * fT * fKi - 8.0f * fKd;
	pxHandle->pfCArr[2] = 2.0f * fT * fKp + fT * fT * fKi + 4.0f * fKd;

	pxHandle->pfCArr[0] /= 2.0f * fT;
	pxHandle->pfCArr[1] /= 2.0f * fT;
	pxHandle->pfCArr[2] /= 2.0f * fT;

	/*	copy handle params	*/
	pxHandle->uiTimeIntervalMs = uiTimeIntervalMs;
	pxHandle->pfGetSample = pfGetSample;
	pxHandle->pfUpdate = pfUpdate;
	pxHandle->pfXPrev[0] = 0.0f;
	pxHandle->pfXPrev[1] = 0.0f;
	pxHandle->pfYPrev[0] = 0.0f;
	pxHandle->pfYPrev[1] = 0.0f;
	pxHandle->fSetPoint = fSetPointInitial;

	/*	Create task	*/
	static uint8_t ucCreatedObjectsCount = 0;
	char pcTaskName[7];
	sprintf(pcTaskName, "PID%d", ucCreatedObjectsCount++);

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
__attribute__((always_inline))
void vHOS_PID_writeSetPoint(xHOS_PID_t* pxHandle, float fSetPoint)
{
	pxHandle->fSetPoint = fSetPoint;
}

/*
 * See header file for info.
 */
__attribute__((always_inline))
void vHOS_PID_enable(xHOS_PID_t* pxHandle)
{
	vTaskResume(pxHandle->xTask);
}

/*
 * See header file for info.
 */
__attribute__((always_inline))
void vHOS_PID_disable(xHOS_PID_t* pxHandle)
{
	vTaskSuspend(pxHandle->xTask);
}
