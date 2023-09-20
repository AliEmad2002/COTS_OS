/*
 * main_template.c
 *
 *  Created on: Sep 20, 2023
 *      Author: Ali Emad
 */

#if 0
/*	LIB	*/
#include <stdint.h>
#include <string.h>

/*	RTOS	*/
#include "FreeRTOS.h"
#include "task.h"

/*	MCAL (Ported)	*/
#include "MCAL_Port/Port_HW.h"
#include "MCAL_Port/Port_Clock.h"

/*	HAL_OS	*/
#include "HAL/HAL_OS.h"

/*******************************************************************************
 * Tasks stacks and handles (initialization task and main tasks):
 ******************************************************************************/
static StackType_t puxTask1Stack[configMINIMAL_STACK_SIZE];
static StaticTask_t xTask1StaticHandle;
static TaskHandle_t xTask1Handle;

static StackType_t puxTaskInitStack[configMINIMAL_STACK_SIZE];
static StaticTask_t xTaskInitStaticHandle;
static TaskHandle_t xTaskInitHandle;

/*******************************************************************************
 * Function prototypes:
 ******************************************************************************/
static void init_main_tasks(void);

/*******************************************************************************
 * Global variables:
 ******************************************************************************/

/*******************************************************************************
 * Task functions:
 ******************************************************************************/
static void vInitTask(void* pvParams)
{
	/*	Initialize MCU's hardware	*/
	vPort_HW_init();

	/*	Initialize COTS_OS	*/

	/*	Initialize main tasks	*/
	init_main_tasks();

	vTaskDelete(NULL);
	while(1);
}

static void vMainTask1(void* pvParams)
{

	while(1)
	{

	}
}


/*******************************************************************************
 * Callbacks:
 ******************************************************************************/
void Error_Handler(void)
{
	while(1);
}

void vApplicationIdleHook( void )
{
   for( ;; )
   {

   }
}

/*******************************************************************************
 * Override HAL weak functions (To work properly with the RTOS):
 ******************************************************************************/
void HAL_Delay(uint32_t Delay)
{
	vTaskDelay(pdMS_TO_TICKS(Delay));
}

/*******************************************************************************
 * Tasks initialization (initialization task and main tasks):
 ******************************************************************************/
static void init_initialization_task(void)
{
	xTaskInitHandle = xTaskCreateStatic(
		vInitTask,
		"initTask",
		configMINIMAL_STACK_SIZE,
		NULL,
		configHOS_INIT_REAL_TIME_TASK_PRI,
		puxTaskInitStack,
		&xTaskInitStaticHandle);
}

static void init_main_tasks(void)
{
	xTask1Handle = xTaskCreateStatic(
		vMainTask1,
		"mainTask1",
		configMINIMAL_STACK_SIZE,
		NULL,
		configHOS_SOFT_REAL_TIME_TASK_PRI,
		puxTask1Stack,
		&xTask1StaticHandle);
}

/*******************************************************************************
 * Main:
 ******************************************************************************/
int main(void)
{
	vPort_Interrupt_disableGlobalInterrupt();

	/*	Initialize clock	*/
	vPort_Clock_init();

	/*	Initialize initialization task	*/
	init_initialization_task();

	/*	Run scheduler	*/
	vTaskStartScheduler();

	vPort_Interrupt_enableGlobalInterrupt();

	while(1)
	{

	}

	return 0;
}

#endif
