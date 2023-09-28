/*
 * RunTimeStackAlalyzer.c
 *
 *  Created on: Sep 25, 2023
 *      Author: Ali Emad
 */

/*	LIB	*/
#include "stdint.h"
#include "string.h"
#include "LIB/Assert.h"

/*	RTOS	*/
#include "FreeRTOS.h"
#include "task.h"
#include "RTOS_PRI_Config.h"

/*	SELF	*/
#include "LIB/RunTimeStackAlalyzer/RunTimeStackAlalyzer.h"

#if ucCONF_RUN_TIME_STACK_ALALYZER_ENABLE

/*******************************************************************************
 * Driver's structure:
 ******************************************************************************/
typedef struct{
	TaskHandle_t xTask;
	char* pcTaskName;
	uint32_t uiMinFreeStackSpace;
}xLIB_RunTimeStackAlalyzer_t;

/*******************************************************************************
 * Driver's static variables (data):
 ******************************************************************************/
static xLIB_RunTimeStackAlalyzer_t pxStackAnalysisArr[uiCONF_RUN_TIME_STACK_ALALYZER_MAX_NUMBER_OF_TASKS];
static uint32_t uiNumberOfAddedTasks = 0;

static StaticTask_t xTaskStatic;
static TaskHandle_t xTask;
static StackType_t xTaskStack[configMINIMAL_STACK_SIZE];

/*******************************************************************************
 * Helping functions / macros:
 ******************************************************************************/


/*******************************************************************************
 * RTOS task:
 ******************************************************************************/
static void vTask(void* pvParams)
{
	while(1)
	{
		for (uint32_t i = 0; i < uiNumberOfAddedTasks; i++)
		{
			/*	Update free stack space	*/
			pxStackAnalysisArr[i].uiMinFreeStackSpace =
				uxTaskGetStackHighWaterMark(pxStackAnalysisArr[i].xTask);

			/*	Check for overflow	*/
			vLib_ASSERT(pxStackAnalysisArr[i].uiMinFreeStackSpace > 0, 1);
		}
	}
}

/*******************************************************************************
 * API functions:
 ******************************************************************************/
/*
 * See header for info.
 */
void vLIB_RunTimeStackAlalyzer_init(void)
{
	/*	Initialize task	*/
	xTask = xTaskCreateStatic(	vTask,
								"RTSA",
								configMINIMAL_STACK_SIZE,
								NULL,
								configHOS_SOFT_REAL_TIME_TASK_PRI,
								xTaskStack,
								&xTaskStatic	);

	/*	Add task to the stack analyzer	*/
	vLIB_RunTimeStackAlalyzer_addTask(xTask);
}

/*
 * See header for info.
 */
void vLIB_RunTimeStackAlalyzer_addTask(TaskHandle_t _xTask)
{
	vLib_ASSERT(uiNumberOfAddedTasks < uiCONF_RUN_TIME_STACK_ALALYZER_MAX_NUMBER_OF_TASKS, 1);

	xLIB_RunTimeStackAlalyzer_t* pxHandle = &pxStackAnalysisArr[uiNumberOfAddedTasks];

	pxHandle->xTask = _xTask;
	pxHandle->pcTaskName = pcTaskGetName(_xTask);
	pxHandle->uiMinFreeStackSpace = uxTaskGetStackHighWaterMark(pxHandle->xTask);

	uiNumberOfAddedTasks++;
}


#endif















