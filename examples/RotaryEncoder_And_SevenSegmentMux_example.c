/*
 * RotaryEncoder_And_SevenSegmentMux_example.c
 *
 *  Created on: Jul 3, 2023
 *      Author: Ali Emad
 *
 * This is an example code for "HAL/RotaryEncoder" and "HAL/SevenSegmentMux" drivers.
 */

/*
 * Un-comment the following definition to use this example.
 * Make sure there's no other main in your program.
 */
#define ROTARY_SS_EXAMPLE

#ifdef ROTARY_SS_EXAMPLE

/*	LIB	*/
#include <stdint.h>

/*	MCAL	*/
#include "MCAL_Port/Port_HW.h"

/*	OS	*/
#include "FreeRTOS.h"
#include "task.h"

/*	HAL_OS	*/
#include "HAL/HAL_OS.h"

/*******************************************************************************
 * Global variables/objects:
 ******************************************************************************/
static xHOS_SevenSegmentMux_t xSevenSegmentMux;
static xHOS_RotaryEncoder_t xRotary;
static int8_t ucCounter = 0;
//xHOS_Button_t xButton;

/*******************************************************************************
 * Tasks stacks and handles:
 ******************************************************************************/
static StackType_t puxTask1Stack[configMINIMAL_STACK_SIZE];
static StaticTask_t xTask1StaticHandle;
static TaskHandle_t xTask1Handle;

/*******************************************************************************
 * Task functions:
 ******************************************************************************/
void vTask1(void* pvParams)
{
	vHOS_SevenSegmentMux_Enable(&xSevenSegmentMux);
	vHOS_RotaryEncoder_enable(&xRotary);

	TickType_t xLastWakeTime = xTaskGetTickCount();

	while(1)
	{
		vHOS_SevenSegmentMux_write(&xSevenSegmentMux, ucCounter, -1);

		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(20));
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
void vCWCallback(void* pvParams)
{
	ucCounter++;
	if (ucCounter > 99)
		ucCounter = 0;
}

void vCCWCallback(void* pvParams)
{
	ucCounter--;
	if (ucCounter < 0)
		ucCounter = 99;
}

/*******************************************************************************
 * Tasks initialization:
 ******************************************************************************/
void tasks_init(void)
{
	xTask1Handle = xTaskCreateStatic(vTask1, "T1", configMINIMAL_STACK_SIZE, NULL, configHOS_SOFT_REAL_TIME_TASK_PRI, puxTask1Stack, &xTask1StaticHandle);
}

/*******************************************************************************
 * HAL_OS objects initialization:
 ******************************************************************************/
void obj_init(void)
{
	/*	Seven segment mux	*/
	static const uint8_t pucSegPortArr[8] = {1 , 1 , 1 , 1 , 1 , 1 , 1 , 1};
	static const uint8_t pucSegPinArr[8]  = {10, 11, 13, 14, 15, 1 , 0 , 12};
	static const uint8_t pucEnPortArr[2] = {0, 0};
	static const uint8_t pucEnPinArr[2]  = {6, 7};
	static uint8_t pucBuffer[2];

	xSevenSegmentMux.pxSegmentPortNumberArr = (uint8_t*)pucSegPortArr;
	xSevenSegmentMux.pxSegmentPinNumberArr = (uint8_t*)pucSegPinArr;
	xSevenSegmentMux.pxDigitEnablePortNumberArr = (uint8_t*)pucEnPortArr;
	xSevenSegmentMux.pxDigitEnablePinNumberArr = (uint8_t*)pucEnPinArr;
	xSevenSegmentMux.ucSegmentActiveLevel = 1;
	xSevenSegmentMux.ucEnableActiveLevel = 0;
	xSevenSegmentMux.ucNumberOfDigits = 2;
	xSevenSegmentMux.pucDisplayBuffer = pucBuffer;
	xSevenSegmentMux.uiUpdatePeriodMs = 10;

	vHOS_SevenSegmentMux_init(&xSevenSegmentMux);

	/*	Rotary encoder	*/
	xRotary.ucAPort = 0;
	xRotary.ucAPin = 4;
	xRotary.ucBPort = 0;
	xRotary.ucBPin = 5;
	xRotary.uiSamplePeriodMs = 5;
	xRotary.uiIdleTimeoutMs = 250;
	xRotary.ucNFilter = 2;
	xRotary.pfCWCallback = vCWCallback;
	xRotary.pfCCWCallback = vCCWCallback;
	vHOS_RotaryEncoder_init(&xRotary);
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
