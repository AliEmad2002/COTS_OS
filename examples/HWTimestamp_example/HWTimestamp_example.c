/*
 * HWTimestamp_example.c
 *
 *  Created on: Aug 29, 2023
 *      Author: Ali Emad
 *
 * This example proofs importance of the method used in "ulHOS_HWTimestamp_getTimestamp()".
 *
 * It runs one task to take a timestamp periodically, and compare it with the previous
 * timestamp. Logically, the new timestamp must be larger than the previous one.
 *
 * Has no accessing faults occured, the new timestamp may be smaller than the
 * previous one. The task should detect this fault and tell if the "HWTimestamp"
 * driver has failed or not.
 *
 * Another task (task2) is of higher priority and does random delays, to emulate
 * other tasks running in the system.
 *
 * When tested on STM32F103C8T6 @72MHz F_CPU, 500KHz F_TIM, the fault never occured.
 *
 * Importance of the used method is noticed by commenting it and using the straight
 * forward method instead. Then, the fault occures.
 */
#if 0

/*	LIB	*/
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*	OS	*/
#include "FreeRTOS.h"
#include "task.h"

/*	MCAL	*/
#include "MCAL_Port/Port_HW.h"

/*	HAL_OS	*/
#include "HAL/HAL_OS.h"

/*******************************************************************************
 * Macros:
 ******************************************************************************/


/*******************************************************************************
 * Global variables/objects:
 ******************************************************************************/
static volatile uint32_t uiTimestamp = 0;
static volatile uint32_t uiTimestampPrev = 0;


/*******************************************************************************
 * Tasks stacks and handles:
 ******************************************************************************/
static StackType_t puxTask1Stack[configMINIMAL_STACK_SIZE];
static StaticTask_t xTask1StaticHandle;
static TaskHandle_t xTask1Handle;

static StackType_t puxTask2Stack[configMINIMAL_STACK_SIZE];
static StaticTask_t xTask2StaticHandle;
static TaskHandle_t xTask2Handle;

/*******************************************************************************
 * Task functions:
 ******************************************************************************/
void vTask1(void* pvParams)
{
	while(1)
	{
		uiTimestampPrev = uiTimestamp;
		uiTimestamp = ulHOS_HWTimestamp_getTimestamp();

		if (uiTimestamp < uiTimestampPrev)
		{
			while(1);	//	fault occurred.
		}
	}
}

void vTask2(void* pvParams)
{

	while(1)
	{
		vTaskSuspend(xTask1Handle);
		vTaskDelay(pdMS_TO_TICKS(abs(rand())%100));
		vTaskResume(xTask1Handle);
		vTaskDelay(pdMS_TO_TICKS(abs(rand())%100));
	}
}

void vApplicationIdleHook( void )
{

   for( ;; )
   {

   }
}

/*******************************************************************************
 * Callbacks:
 ******************************************************************************/


/*******************************************************************************
 * Tasks initialization:
 ******************************************************************************/
void tasks_init(void)
{
	xTask1Handle = xTaskCreateStatic(
		vTask1,
		"",
		configMINIMAL_STACK_SIZE,
		NULL,
		configHOS_SOFT_REAL_TIME_TASK_PRI,
		puxTask1Stack,
		&xTask1StaticHandle);

	xTask2Handle = xTaskCreateStatic(
		vTask2,
		"",
		configMINIMAL_STACK_SIZE,
		NULL,
		configHOS_HARD_REAL_TIME_TASK_PRI,
		puxTask2Stack,
		&xTask2StaticHandle);
}

/*******************************************************************************
 * HAL_OS objects initialization:
 ******************************************************************************/
void obj_init(void)
{
	vHOS_HWTimestamp_init();
}

/*******************************************************************************
 * main:
 ******************************************************************************/
int main(void)
{
	/*	init	*/
	vPort_HW_init();
	obj_init();
	tasks_init();

	/*	Run	*/
	vTaskStartScheduler();

	while(1)
	{

	}

	return 0;
}

#endif
