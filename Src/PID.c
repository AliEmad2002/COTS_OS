/*
 * PID.c
 *
 *  Created on: Jun 28, 2023
 *      Author: Ali Emad
 */

/*	LIB	*/
#include <stdint.h>
#include <stdio.h>

/*	FreeRTOS	*/
#include "FreeRTOS.h"
#include "task.h"

/*	HAL-OS	*/
#include "Inc/RTOS_PRI_Config.h"

/*	SELF	*/
#include "Inc/PID/PID.h"

/*******************************************************************************
 * Helping functions/macros:
 ******************************************************************************/
#define vCLAMP(fVal, fMin, fMax)     \
{                                    \
	if ((fVal) < (fMin))             \
		(fVal) = (fMin);             \
	else if ((fVal) > (fMax))        \
		(fVal) = (fMax);             \
}

/*******************************************************************************
 * Task function
 ******************************************************************************/
static void vTask(void* pvParams)
{
	xHOS_PID_t* pxHandle = (xHOS_PID_t*)pvParams;
	vTaskSuspend(pxHandle->xTask);

	TickType_t xLastWakeTime = xTaskGetTickCount();
	float fENew;
	float fP;
	float fSum;
	float fC0, fC1;
	while(1)
	{
		/*	Get new error value	*/
		fENew = pxHandle->fSetPoint - pxHandle->pfGetSample();

		/*	calculate and clamp integral term	*/
		fC0 = (pxHandle->fKi * pxHandle->uiTimeIntervalMs) / 2000.0f;
		pxHandle->fI = pxHandle->fI + fC0 * (pxHandle->fE + fENew);
		vCLAMP(pxHandle->fI, pxHandle->fIMin, pxHandle->fIMax);

		/*	calculate and clamp derivative term	*/
		fC1 = (2000.0f * pxHandle->fKd) / pxHandle->uiTimeIntervalMs;
		pxHandle->fD = fC1 * (fENew - pxHandle->fE);
		vCLAMP(pxHandle->fD, pxHandle->fDMin, pxHandle->fDMax);

		/*	calculate proportional term	*/
		fP = pxHandle->fKp * fENew;

		/*	Sum  and clamp summation	*/
		fSum = pxHandle->fI + pxHandle->fD + fP;
		vCLAMP(fSum, pxHandle->fOutMin, pxHandle->fOutMax);

		/*	update plant with the new sum value	*/
		pxHandle->pfUpdate(fSum);

		/*	update controller variables	*/
		pxHandle->fE = fENew;

		/*	Task is blocked until next sample time	*/
		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(pxHandle->uiTimeIntervalMs));
	}
}

/*******************************************************************************
 * API functions
 ******************************************************************************/
/*
 * See header file for info.
 */
void vHOS_PID_init(xHOS_PID_t* pxHandle)
{
	/*	Initialize handle variables	*/
	pxHandle->fD = 0.0f;
	pxHandle->fI = 0.0f;
	pxHandle->fE = 0.0f;

	/*	Create task	*/
	static uint8_t ucCreatedObjectsCount = 0;
	char pcTaskName[configMAX_TASK_NAME_LEN];
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
__attribute__((always_inline)) inline
void vHOS_PID_enable(xHOS_PID_t* pxHandle)
{
	vTaskResume(pxHandle->xTask);
}

/*
 * See header file for info.
 */
__attribute__((always_inline)) inline
void vHOS_PID_disable(xHOS_PID_t* pxHandle)
{
	vTaskSuspend(pxHandle->xTask);
}
