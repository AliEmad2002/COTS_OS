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

#if 0

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

#define MAX_VAL ((1ul << 31) - 1)

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
static uint8_t is_element_in_arr(int32_t iElem, int32_t piArr[], uint32_t uiArrLen);
static void sort_arr(int32_t piArr[], int32_t iArrLen);

/*******************************************************************************
 * Global variables:
 ******************************************************************************/
xLIB_Heap_t xHeap;
int32_t piElemArr[xCONF_VECTOR_MAX_SIZE];
volatile uint32_t uiArrSize;
volatile uint32_t uiNumberOfElementsToRemove;
volatile uint32_t uiSuccessCount = 0;
volatile int32_t iParent, iLeftChild, iRightChild;

/*******************************************************************************
 * Task functions:
 ******************************************************************************/
static void vInitTask(void* pvParams)
{
	/*	Initialize MCU's hardware	*/
	vPort_HW_init();

	srand(100);

	/*	Initialize COTS_OS	*/
	vLIB_Heap_init(&xHeap);

	/*	Initialize main tasks	*/
	init_main_tasks();

	vTaskDelete(NULL);
	while(1);
}

static void vMainTask1(void* pvParams)
{
	while(1)
	{
		/*	Give the array a random length	*/
		uiArrSize = abs(rand()) % (xCONF_VECTOR_MAX_SIZE);

		/*	Fill the array with random numbers	*/
		for (uint32_t i = 0; i < uiArrSize; i++)
			piElemArr[i] = abs(rand()) % MAX_VAL;

		/*	Insert array to the heap	*/
		for (uint32_t i = 0; i < uiArrSize; i++)
			vLIB_Heap_addElem(&xHeap, (int32_t*)&piElemArr[i]);

		/*	Sort the array	*/
		sort_arr(piElemArr, uiArrSize);

		/*	Remove random amount of elements from the heap	*/
		uiNumberOfElementsToRemove = abs(rand()) % (uiArrSize + 1);
		for (uint32_t i = 0; i < uiNumberOfElementsToRemove; i++)
			vLIB_Heap_removeTop(&xHeap);

		/*	Remove same elements from the array	*/
		uiArrSize -= uiNumberOfElementsToRemove;

		/*	Validate size	*/
		vLib_ASSERT(uiLIB_HEAP_GET_SIZE(&xHeap) == uiArrSize, 2);

		/*	Validate that each element of the heap is available in the array	*/
		for (uint32_t i = 1; i < uiArrSize + 1; i++)
		{
			vLib_ASSERT(is_element_in_arr(xHeap.xTree.xVector.pxArr[i], piElemArr, uiArrSize), 2);
		}

		/*	Validate that each element of the array is available in the heap	*/
		for (uint32_t i = 0; i < uiArrSize; i++)
		{
			vLib_ASSERT(is_element_in_arr(piElemArr[i], xHeap.xTree.xVector.pxArr, uiArrSize+1), 2);
		}

		/*	Validate heap nodes' priorities	*/
		for (uint32_t i = 1; i < uiArrSize + 1; i++)
		{
			iParent = xHeap.xTree.xVector.pxArr[i];

			if (ucLIB_COMPLETE_BINARY_TREE_HAS_LEFT(&xHeap.xTree, &xHeap.xTree.xVector.pxArr[i]))
			{
				iLeftChild = xHeap.xTree.xVector.pxArr[2 * i];
				vLib_ASSERT(ucCONF_HEAP_COMPARE(iParent, iLeftChild), 2);
			}

			if (ucLIB_COMPLETE_BINARY_TREE_HAS_RIGHT(&xHeap.xTree, &xHeap.xTree.xVector.pxArr[i]))
			{
				iRightChild = xHeap.xTree.xVector.pxArr[2 * i + 1];
				vLib_ASSERT(ucCONF_HEAP_COMPARE(iParent, iRightChild), 2);
			}
		}

		/*	Clear heap for next iteration	*/
		while(uiLIB_HEAP_GET_SIZE(&xHeap) > 0)
			vLIB_Heap_removeTop(&xHeap);

		uiSuccessCount++;
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
 * Helping functions:
 ******************************************************************************/
static uint8_t is_element_in_arr(int32_t iElem, int32_t piArr[], uint32_t uiArrLen)
{
	for (uint32_t i = 0; i < uiArrLen; i++)
	{
		if (piArr[i] == iElem)
			return 1;
	}
	return 0;
}

static void sort_arr(int32_t piArr[], int32_t iArrLen)
{
	for (int32_t i = 0; i < iArrLen - 1; i++)
	{
		for (int32_t j = i + 1; j < iArrLen; j++)
		{
			if (piArr[i] < piArr[j])
			{
				int32_t _temp = piArr[i];
				piArr[i] = piArr[j];
				piArr[j] = _temp;
			}
		}
	}
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
