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

/*	MCAL	*/
#include "Port/Port_HW.h"

/*	OS	*/
#include "FreeRTOS.h"
#include "task.h"

/*	MCAL	*/
#include "stm32f103xb.h"
#include "stm32f1xx_hal.h"

/*	HAL_OS	*/
#include "Inc/HAL_OS.h"

/*
 * Task functions:
 */
void vTask1(void* pvParams)
{
	while(1)
	{
		HAL_GPIO_TogglePin(GPIOB, 1 << 12);
		vTaskDelay(pdMS_TO_TICKS(500));
	}
}

void vTask2(void* pvParams)
{
	while(1)
	{
		HAL_GPIO_TogglePin(GPIOB, 1 << 13);
		vTaskDelay(pdMS_TO_TICKS(800));
	}
}

/*
 * Tasks stacks and handles:
 */
static StackType_t puxTask1Stack[configMINIMAL_STACK_SIZE];
static StaticTask_t xTask1StaticHandle;
static TaskHandle_t xTask1Handle;
static StackType_t puxTask2Stack[configMINIMAL_STACK_SIZE];
static StaticTask_t xTask2StaticHandle;
static TaskHandle_t xTask2Handle;

/*
 * Button callback:
 */
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

/*
 * HW initialization:
 */
void HW_init(void)
{
	/*	RCC	*/
	SystemCoreClockUpdate();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/*	Output1	*/
	GPIO_InitTypeDef conf1 = {
		.Pin = 1 << 12,
		.Mode = GPIO_MODE_OUTPUT_PP,
		.Pull = GPIO_NOPULL,
		.Speed = GPIO_SPEED_FREQ_HIGH
	};
	HAL_GPIO_Init(GPIOB, &conf1);

	/*	Output2	*/
	GPIO_InitTypeDef conf2 = {
		.Pin = 1 << 13,
		.Mode = GPIO_MODE_OUTPUT_PP,
		.Pull = GPIO_NOPULL,
		.Speed = GPIO_SPEED_FREQ_HIGH
	};
	HAL_GPIO_Init(GPIOB, &conf2);
}

/*
 * Tasks initialization:
 */
void tasks_init(void)
{
	xTask1Handle = xTaskCreateStatic(vTask1, "T1", configMINIMAL_STACK_SIZE, NULL, configNORMAL_TASK_PRIORITY, puxTask1Stack, &xTask1StaticHandle);
	xTask2Handle = xTaskCreateStatic(vTask2, "T2", configMINIMAL_STACK_SIZE, NULL, configNORMAL_TASK_PRIORITY, puxTask2Stack, &xTask2StaticHandle);
}

/*
 * Button initialization:
 */
void button_init(void)
{
	(void)pxHOS_Button_init(1, 1, callback, 1, 3);
}

int main(void)
{
	/*	init	*/
	vPort_HW_init();
	HW_init();
	button_init();
	tasks_init();
	xHOS_init();

	/*	Run	*/
	vTaskStartScheduler();

	while(1)
	{

	}

	return 0;
}
#endif
