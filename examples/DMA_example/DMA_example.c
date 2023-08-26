/*
 * DMA_Example.c
 *
 *  Created on: Aug 26, 2023
 *      Author: Ali Emad
 *
 * In this example, number of tasks copy and check random data from one array to
 * the other. Each task has its own arrays. they use  the DMA driver to lock the
 * first available DMA channel, copy, and release the channel.
 *
 * An additional task does the same process but it acquires certain channel (to
 * emulate peripherals which do so).
 *
 * "uiChannel[]" can be monitored in run time to observe how tasks share DMA
 * channels. As "uiChannel[i]" has a value equal to number of the DMA channel
 * being currently used by i-th task.
 */
#if 0

/*	LIB	*/
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

/*	OS	*/
#include "FreeRTOS.h"
#include "task.h"

/*	MCAL	*/
#include "MCAL_Port/Port_HW.h"
#include "MCAL_Port/Port_DMA.h"

/*	HAL_OS	*/
#include "HAL/HAL_OS.h"

/*******************************************************************************
 * Macros:
 ******************************************************************************/
#define uiSIZE		350
#define ucNUMBER_OF_TASKS	10

/*******************************************************************************
 * Global variables/objects:
 ******************************************************************************/
static uint32_t  uiChannel[ucNUMBER_OF_TASKS+1];

static volatile uint8_t pucArr1[ucNUMBER_OF_TASKS+1][uiSIZE];
static volatile uint8_t pucArr2[ucNUMBER_OF_TASKS+1][uiSIZE];

/*******************************************************************************
 * Tasks stacks and handles:
 ******************************************************************************/
static StackType_t puxTaskStack[ucNUMBER_OF_TASKS+1][configMINIMAL_STACK_SIZE];
static StaticTask_t xTaskStaticHandle[ucNUMBER_OF_TASKS+1];
static TaskHandle_t xTaskHandleArr[ucNUMBER_OF_TASKS+1];

/*******************************************************************************
 * Task functions:
 ******************************************************************************/
void vTask1(void* pvParams)
{
	uint8_t ucTaskNumber = (uint8_t)((uint32_t)pvParams);
	uiChannel[ucTaskNumber] = 0;

	/*	Prepare transfer info	*/
	xHOS_DMA_TransInfo_t xInfo = {
		.ucUnitNumber = 0,	// any (not known yet).
		.ucChannelNumber = 0,	// any (not known yet).
		.pvMemoryStartingAdderss = (void*)pucArr1[ucTaskNumber],
		.pvPeripheralStartingAdderss = (void*)pucArr2[ucTaskNumber],
		.uiN = uiSIZE,
		.ucTriggerSource = 1,
		.ucPriLevel = 0,
		.ucDirection = 1,
		.ucMemoryIncrement = 1,
		.ucPeripheralIncrement = 1,
	};

	for (int32_t i = 0; i < uiSIZE; i++)
	{
		pucArr1[ucTaskNumber][i] = abs(rand()) % ((1ul << 31) - 1);
	}

	while(1)
	{
		if (abs(rand()) % 2)
			vTaskDelay(abs(rand()) % 100);

		/*	Lock the first available DMA channel	*/
		ucHOS_DMA_lockAnyChannel(&xInfo.ucUnitNumber, &xInfo.ucChannelNumber, portMAX_DELAY);

		uiChannel[ucTaskNumber] = xInfo.ucChannelNumber;

		/*	Start transfer	*/
		vHOS_DMA_clearTransferCompleteFlag(xInfo.ucUnitNumber, xInfo.ucChannelNumber);
		vHOS_DMA_startTransfer(&xInfo);

		/*	Block until transfer complete	*/
		ucHOS_DMA_blockUntilTransferComplete(xInfo.ucUnitNumber, xInfo.ucChannelNumber, portMAX_DELAY);

		/*	Release DMA channel	*/
		ucHOS_DMA_releaseChannel(xInfo.ucUnitNumber, xInfo.ucChannelNumber, portMAX_DELAY);

		/*	Validate transfer	*/
		for (int32_t i = 0; i < uiSIZE; i++)
		{
			if (pucArr1[ucTaskNumber][i] != pucArr2[ucTaskNumber][i])
			{
				while(1);
			}
		}
	}
}

void vTask2(void* pvParams)
{
	uint8_t ucTaskNumber = (uint8_t)((uint32_t)pvParams);
	uiChannel[ucTaskNumber] = 0;

	/*	Prepare transfer info	*/
	xHOS_DMA_TransInfo_t xInfo = {
		.ucUnitNumber = 0,
		.ucChannelNumber = 5,
		.pvMemoryStartingAdderss = (void*)pucArr1[ucTaskNumber],
		.pvPeripheralStartingAdderss = (void*)pucArr2[ucTaskNumber],
		.uiN = uiSIZE,
		.ucTriggerSource = 1,
		.ucPriLevel = 0,
		.ucDirection = 1,
		.ucMemoryIncrement = 1,
		.ucPeripheralIncrement = 1,
	};

	for (int32_t i = 0; i < uiSIZE; i++)
	{
		pucArr1[ucTaskNumber][i] = abs(rand()) % ((1ul << 31) - 1);
	}

	while(1)
	{
		if (abs(rand()) % 2)
			vTaskDelay(abs(rand()) % 100);

		/*	Lock the first available DMA channel	*/
		uint8_t ucState = ucHOS_DMA_lockChannel(xInfo.ucUnitNumber, xInfo.ucChannelNumber, portMAX_DELAY);
		if (ucState == 0)
			continue;

		uiChannel[ucTaskNumber] = xInfo.ucChannelNumber;

		/*	Start transfer	*/
		vHOS_DMA_clearTransferCompleteFlag(xInfo.ucUnitNumber, xInfo.ucChannelNumber);
		vHOS_DMA_startTransfer(&xInfo);

		/*	Block until transfer complete	*/
		ucHOS_DMA_blockUntilTransferComplete(xInfo.ucUnitNumber, xInfo.ucChannelNumber, portMAX_DELAY);

		/*	Release DMA channel	*/
		ucHOS_DMA_releaseChannel(xInfo.ucUnitNumber, xInfo.ucChannelNumber, portMAX_DELAY);

		/*	Validate transfer	*/
		for (int32_t i = 0; i < uiSIZE; i++)
		{
			if (pucArr1[ucTaskNumber][i] != pucArr2[ucTaskNumber][i])
			{
				while(1);
			}
		}
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
	for (uint8_t i = 0; i < ucNUMBER_OF_TASKS; i++)
	{
		xTaskHandleArr[i] = xTaskCreateStatic(
			vTask1,
			"",
			configMINIMAL_STACK_SIZE,
			(void*)((uint32_t)i),
			configHOS_SOFT_REAL_TIME_TASK_PRI,
			puxTaskStack[i],
			&xTaskStaticHandle[i]);
	}

	xTaskHandleArr[ucNUMBER_OF_TASKS] = xTaskCreateStatic(
		vTask2,
		"",
		configMINIMAL_STACK_SIZE,
		(void*)ucNUMBER_OF_TASKS,
		configHOS_SOFT_REAL_TIME_TASK_PRI,
		puxTaskStack[ucNUMBER_OF_TASKS],
		&xTaskStaticHandle[ucNUMBER_OF_TASKS]);

}

/*******************************************************************************
 * HAL_OS objects initialization:
 ******************************************************************************/
void obj_init(void)
{
	vHOS_DMA_init();
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
