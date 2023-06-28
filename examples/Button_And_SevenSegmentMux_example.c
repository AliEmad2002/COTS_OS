/*
 * Button_And_SevenSegmentMux_example.c
 *
 *  Created on: Jun 16, 2023
 *      Author: Ali Emad
 *
 * This is an example code for "HAL_OS/Button" and "HAL_OS/SevenSegmentMux" drivers.
 *
 * There would be a seven segment display of two digits, showing a counter that
 * increments every 0.5 second.
 *
 * Button press would toggle the display on/off.
 *
 * Ported and configured for target: STM32F103C8x
 */

/*
 * Un-comment the following definition to use this example.
 * Make sure there's no other main in your program.
 */
//#define BUTTON_SS_EXAMPLE

#ifdef BUTTON_SS_EXAMPLE

/*	LIB	*/
#include <stdint.h>

/*	MCAL	*/
#include "Port/Port_HW.h"

/*	OS	*/
#include "FreeRTOS.h"
#include "task.h"

/*	HAL_OS	*/
#include "Inc/HAL_OS.h"

/*******************************************************************************
 * Global variables/objects:
 ******************************************************************************/
xHOS_SevenSegmentMux_t* pxSevenSegmentMux;

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
	while(1)
	{
		for (uint8_t i = 0; i < 100; i++)
		{
			vHOS_SevenSegmentMux_write(pxSevenSegmentMux, i, 0);
			vTaskDelay(pdMS_TO_TICKS(500));
		}
	}
}

/*******************************************************************************
 * Callbacks:
 ******************************************************************************/
void vButtonCallback(void)
{
	static uint8_t isSevenSegmentEn = 0;

	if (isSevenSegmentEn == 0)
	{
		vHOS_SevenSegmentMux_Enable(pxSevenSegmentMux);
		isSevenSegmentEn = 1;
	}

	else
	{
		vHOS_SevenSegmentMux_Disable(pxSevenSegmentMux);
		isSevenSegmentEn = 0;
	}
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
	static const uint8_t pucSegPortArr[8] = {1 , 1 , 1 , 1 , 1 , 1 , 1 , 1};
	static const uint8_t pucSegPinArr[8]  = {10, 11, 13, 14, 15, 1 , 0 , 12};
	static const uint8_t pucEnPortArr[2] = {0, 0};
	static const uint8_t pucEnPinArr[2]  = {6, 7};

	pxSevenSegmentMux = pxHOS_SevenSegmentMux_init(	(uint8_t*)pucSegPortArr,
													(uint8_t*)pucSegPinArr,
													(uint8_t*)pucEnPortArr,
													(uint8_t*)pucEnPinArr,
													1, 0, 2	);

	xHOS_Button_t* pxButton = pxHOS_Button_init(1, 9, vButtonCallback, 1, 4);
	vHOS_Button_Enable(pxButton);
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
