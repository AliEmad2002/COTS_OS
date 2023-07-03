/*
 * HardwareDelay_example.c
 *
 *  Created on: Jun 26, 2023
 *      Author: Ali Emad
 */

/*
 * Un-comment the following definition to use this example.
 * Make sure there's no other main in your program.
 */
//#define HARDWARE_DELAY_EXAMPLE_1

#ifdef HARDWARE_DELAY_EXAMPLE_1
/*	LIB	*/
#include <stdint.h>
#include <stdlib.h>

/*	OS	*/
#include "FreeRTOS.h"
#include "task.h"

/*	MCAL	*/
#include "stm32f103xb.h"
#include "stm32f1xx_hal.h"
#include "MCAL_Port/Port_HW.h"
#include "MCAL_Port/Port_DIO.h"

/*	HAL_OS	*/
#include "Inc/HAL_OS.h"

/*******************************************************************************
 * Global variables/objects:
 ******************************************************************************/
xHOS_HardwareDelay_t* pxHardwareDelayHandle1;
xHOS_HardwareDelay_t* pxHardwareDelayHandle2;

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
	xSemaphoreTake(pxHardwareDelayHandle1->xMutex, portMAX_DELAY);

	vPort_DIO_initPinOutput(1, 12);

	while(1)
	{
		vHOS_HardwareDelay_delayTicks(pxHardwareDelayHandle1, 10000);
		vPort_DIO_togglePin(1, 12);
	}
}

void vTask2(void* pvParams)
{
	xSemaphoreTake(pxHardwareDelayHandle2->xMutex, portMAX_DELAY);

	vPort_DIO_initPinOutput(1, 13);

	while(1)
	{
		vHOS_HardwareDelay_delayTicks(pxHardwareDelayHandle2, 10000);
		vPort_DIO_togglePin(1, 13);
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
 * Callbacks:
 ******************************************************************************/


/*******************************************************************************
 * Tasks initialization:
 ******************************************************************************/
void tasks_init(void)
{
	xTask1Handle = xTaskCreateStatic(vTask1, "T1", configMINIMAL_STACK_SIZE, NULL, configHOS_SOFT_REAL_TIME_TASK_PRI, puxTask1Stack, &xTask1StaticHandle);
	xTask2Handle = xTaskCreateStatic(vTask2, "T2", configMINIMAL_STACK_SIZE, NULL, configHOS_SOFT_REAL_TIME_TASK_PRI-1, puxTask2Stack, &xTask2StaticHandle);
}

/*******************************************************************************
 * HAL_OS objects initialization:
 ******************************************************************************/
void obj_init(void)
{
	pxHardwareDelayHandle1 = pxHOS_HardwareDelay_initNewObject(xHOS_HardwareDelay_Frequency_10kHz);
	pxHardwareDelayHandle2 = pxHOS_HardwareDelay_initNewObject(xHOS_HardwareDelay_Frequency_100kHz);
}

/*******************************************************************************
 * main:
 ******************************************************************************/
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
