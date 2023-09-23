/*
 * Heap_Test.c
 *
 *  Created on: Sep 23, 2023
 *      Author: Ali Emad
 *
 * This is a test that creates random heaps, and validate their data campared to heap's
 * properties of:
 * 		-	Being of a complete binary tree.
 * 		-	Node is of higher priority to be the root node than its children (if any).
 * 		-	All elements in the array are in the heap.
 * 		-	Size of the heap is equal to that of the array.
 *
 * This is very useful to test the "Heap" driver every time its implementation is updated.
 *
 * This test code uses "int32_t" as a "xCONF_VECTOR_ELEM_TYPE".
 */

#if 1

/*	LIB	*/
#include <stdint.h>
#include <string.h>
#include "LIB/Heap/Heap.h"

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
xLIB_Heap_t xHeap;
int32_t piElemArr[xCONF_VECTOR_MAX_SIZE];

/*******************************************************************************
 * Task functions:
 ******************************************************************************/
static void vInitTask(void* pvParams)
{
	/*	Initialize MCU's hardware	*/
	vPort_HW_init();

	/*	Initialize COTS_OS	*/
	vLIB_Heap_init(&xHeap);

	/*	Initialize main tasks	*/
	init_main_tasks();

	vTaskDelete(NULL);
	while(1);
}

static void vMainTask1(void* pvParams)
{
	for (uint32_t i = 0; i < 5; i++)
	{
		vLIB_Heap_addElem(&xHeap, (int32_t*)&piElemArr[i]);
	}

	vLIB_Heap_removeTop(&xHeap);

	while(1)
	{
		/*	Fill the array with random numbers	*/

		/*	Insert array to the heap	*/

		/*	Validate size	*/

		/*	Validate that each element of the heap is available in the array	*/

		/*	Validate that each element of the array is available in the heap	*/

		/*	Validate heap nodes' priorities	*/

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
