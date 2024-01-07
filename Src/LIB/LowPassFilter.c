/*
 * LowPassFilter.c
 *
 *  Created on: Jan 5, 2024
 *      Author: Ali Emad
 */


/*	LIB */
#include "stdint.h"

/*	RTOS	*/
#include "FreeRTOS.h"
#include "task.h"
#include "RTOS_PRI_Config.h"

/*	SELF	*/
#include "LIB/LowPassFilter/LowPassFilter.h"


/*******************************************************************************
 * RTOS task:
 ******************************************************************************/
static void vTask(void* pvParams)
{
	xLIB_LowPassFilter_t* pxHandle = (xLIB_LowPassFilter_t*)pvParams;

	TickType_t xLastWkpTime = xTaskGetTickCount();

	while(1)
	{
		/*	Get new sample	*/
		float fX = pxHandle->pfGetSample();

		/*	Apply filter equation	*/
		pxHandle->fOutput =
				(fX + pxHandle->fXPrev) * pxHandle->pfC[0] -
				pxHandle->fYPrev * pxHandle->pfC[1]	;


		/*	Update prev	*/
		pxHandle->fXPrev = fX;
		pxHandle->fYPrev = pxHandle->fOutput;

		/*	Block until next sample time	*/
		vTaskDelayUntil(&xLastWkpTime, pdMS_TO_TICKS(pxHandle->uiSampleTimeMs));
	}
}

/*******************************************************************************
 * API functions:
 ******************************************************************************/
/*
 * See header for info.
 */
void vLIB_LowPassFilter_init(	xLIB_LowPassFilter_t* pxHandle,
								float fWc,
								float fK	)
{
	/*	Calculate filter's constants	*/
	float fFs = 1000.0f / pxHandle->uiSampleTimeMs;
	pxHandle->pfC[0] = fK * fWc / (fWc + 2.0f * fFs);
	pxHandle->pfC[1] = fK * (fWc - 2.0f * fFs) / (fWc + 2.0f * fFs);

	/*	Initially zeros	*/
	pxHandle->fXPrev = 0;
	pxHandle->fYPrev = 0;

	/*	Create task	*/
	pxHandle->xTask = xTaskCreateStatic(	vTask,
											"LPF",
											uiHOS_LPF_STACK_SZ,
											(void*)pxHandle,
											configHOS_MID_REAL_TIME_TASK_PRI,
											pxHandle->pxTaskStack,
											&pxHandle->xTaskStatic	);
}
