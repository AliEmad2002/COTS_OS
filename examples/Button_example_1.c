/*
 * Button_example_1.c
 *
 *  Created on: Jun 8, 2023
 *      Author: Ali Emad
 *
 * This is an example code for "HAL_OS/Button" driver.
 *
 * There would be two LEDs blinking, each in its own task, and button would toggle
 * LEDs running/suspended state.
 *
 * Ported and configured for target: STM32F103C8x
 */

/*
 * Un-comment the following definition to use this example.
 * Make sure there's no other main in your program.
 */
//#define BUTTON_EXAMPLE_1

#ifdef BUTTON_EXAMPLE_1
/*	LIB	*/
#include <stdint.h>

/*	RTOS	*/
#include "FreeRTOS.h"
#include "task.h"

/*	MCAL (Ported)	*/
#include "Port/Port_HW.h"

/*	HAL_OS	*/
#include "Inc/HAL_OS.h"

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
	vPort_DIO_initPinOutput(1, 12);

	while(1)
	{
		vPort_DIO_togglePin(1, 12);
		vTaskDelay(pdMS_TO_TICKS(500));
	}
}

void vTask2(void* pvParams)
{
	vPort_DIO_initPinOutput(1, 13);

	while(1)
	{
		vPort_DIO_togglePin(1, 13);
		vTaskDelay(pdMS_TO_TICKS(800));
	}
}

void vApplicationIdleHook( void )
{
   for( ;; )
   {
       //vCoRoutineSchedule();
   }
}

/*******************************************************************************
 * Global variables:
 ******************************************************************************/
static xHOS_Button_t xButton;

/*******************************************************************************
 * Callbacks:
 ******************************************************************************/
void callback(void)
{
	static uint8_t areTasksSuspended = 0;

	if(areTasksSuspended)
	{
		vTaskResume(xTask1Handle);
		vTaskResume(xTask2Handle);
		areTasksSuspended = 0;
	}
	else
	{
		vTaskSuspend(xTask1Handle);
		vTaskSuspend(xTask2Handle);
		areTasksSuspended = 1;
	}
}

/*******************************************************************************
 * Tasks initialization:
 ******************************************************************************/
void tasks_init(void)
{
	xTask1Handle = xTaskCreateStatic(vTask1, "T1", configMINIMAL_STACK_SIZE, NULL, configHOS_SOFT_REAL_TIME_TASK_PRI, puxTask1Stack, &xTask1StaticHandle);
	xTask2Handle = xTaskCreateStatic(vTask2, "T2", configMINIMAL_STACK_SIZE, NULL, configHOS_SOFT_REAL_TIME_TASK_PRI, puxTask2Stack, &xTask2StaticHandle);
}

/*******************************************************************************
 * HAL_OS objects initialization:
 ******************************************************************************/
void obj_init(void)
{
	xButton.ucPortNumber = 0;
	xButton.ucPinNumber = 8;
	xButton.pfCallback = callback;
	xButton.ucPressedLevel = 1;
	xButton.ucFilterN = 3;
	xButton.uiSamplePeriodMs = 10;
	vHOS_Button_init(&xButton);
}

int main(void)
{
	/*	init	*/
	vPort_HW_init();
	configASSERT(xHOS_init());
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
