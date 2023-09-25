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

/*******************************************************************************
 * Driver's structure:
 ******************************************************************************/
typedef struct{
	char* pcTaskName;
	StackType_t* pxTaskStack;
	uint32_t uiTaskStackSize;
	uint32_t uiLastCheckStackUsageInWords;
}xLIB_RunTimeStackAlalyzer_t;

/*******************************************************************************
 * Driver's static variables (data):
 ******************************************************************************/
static xLIB_RunTimeStackAlalyzer_t pxTaskAnalysisArr[uiCONF_RUN_TIME_STACK_ALALYZER_MAX_NUMBER_OF_TASKS];
static uint32_t uiNumberOfAddedTasks = 0;

static StaticTask_t xTaskStatic;
static TaskHandle_t xTask;
static StackType_t xTaskStack[configMINIMAL_STACK_SIZE];

/*******************************************************************************
 * Helping functions / macros:
 ******************************************************************************/
void vUpdateStackUsage(xLIB_RunTimeStackAlalyzer_t* pxHandle)
{
	/*	Count number of words filled with the FreeRTOS stack initialization pattern	*/
	uint32_t uiCount = 0;

	for (; uiCount < pxHandle->uiTaskStackSize; uiCount++)
	{
		if (pxHandle->pxTaskStack[uiCount] != 0xA5A5A5A5)
			break;
	}

	/*	Assert stack overflow	*/
	vLib_ASSERT(uiCount <pxHandle->uiTaskStackSize, 1);

	/*	Update	*/
	pxHandle->uiLastCheckStackUsageInWords = pxHandle->uiTaskStackSize - uiCount;
}

/*******************************************************************************
 * RTOS task:
 ******************************************************************************/
static void vTask(void* pvParams)
{
	while(1)
	{
		/*	Update usage of all added tasks	*/
		for (uint32_t i = 0; i < uiNumberOfAddedTasks; i++)
			vUpdateStackUsage(&pxTaskAnalysisArr[i]);
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
	vLIB_RunTimeStackAlalyzer_addTask(	(char*)xTaskStatic.ucDummy7,
										xTaskStack,
										configMINIMAL_STACK_SIZE	);
}

/*
 * See header for info.
 */
void vLIB_RunTimeStackAlalyzer_addTask(	char* pcTaskName,
										StackType_t* pxTaskStack,
										uint32_t uiStackSize	)
{
	vLib_ASSERT(uiNumberOfAddedTasks < uiCONF_RUN_TIME_STACK_ALALYZER_MAX_NUMBER_OF_TASKS, 1);

	xLIB_RunTimeStackAlalyzer_t* pxHandle = &pxTaskAnalysisArr[uiNumberOfAddedTasks];

	pxHandle->pcTaskName = pcTaskName;
	pxHandle->pxTaskStack = pxTaskStack;
	pxHandle->uiTaskStackSize = uiStackSize;
	pxHandle->uiLastCheckStackUsageInWords = 0;

	uiNumberOfAddedTasks++;
}

/*
 * See header for info.
 */
int32_t iLIB_RunTimeStackAlalyzer_getTaskStackUsageInWords(char* pcTaskName)
{
	xLIB_RunTimeStackAlalyzer_t* pxHandle;

	for (uint32_t i = 0; i < uiNumberOfAddedTasks; i++)
	{
		pxHandle = &pxTaskAnalysisArr[i];

		if (strcmp(pcTaskName, pxHandle->pcTaskName) == 0)
		{
			return pxHandle->uiLastCheckStackUsageInWords;
		}
	}

	return -1;
}

















