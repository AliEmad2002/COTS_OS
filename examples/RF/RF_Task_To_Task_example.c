/*
 * RF_Task_To_Task_example.c
 *
 *  Created on: Jul 28, 2023
 *      Author: Ali Emad
 *
 * This is a simple example to test driver's both receiver and transmitter on the
 * same MCU. Simply by transmitting a random set of data, and checking if it is
 * the same data received. And updating a "successRatio" variable to observe driver's
 * operation.
 */

#if 0

/*	LIB	*/
#include <stdint.h>
#include <stdlib.h>

/*	OS	*/
#include "FreeRTOS.h"
#include "task.h"

/*	MCAL	*/
#include "MCAL_Port/Port_HW.h"
#include "MCAL_Port/Port_DIO.h"
#include "MCAL_Port/Port_Breakpoint.h"

/*	HAL_OS	*/
#include "HAL/HAL_OS.h"

/*******************************************************************************
 * Global variables/objects:
 ******************************************************************************/
xHOS_RF_t xRF;
float fSuccessRatio;

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
	volatile uint8_t ucMatch;
	volatile uint32_t uiSuccessCount = 0;
	volatile uint32_t uiTotalCount = 0;
	uint8_t pucData[uiRF_DATA_BYTES_PER_FRAME];

	srand(500);

	vHOS_RF_enable(&xRF);

	TickType_t xLastWakeTime = xTaskGetTickCount();
	while(1)
	{
		/*	Fill data array with random values	*/
		for (uint32_t i = 0; i < uiRF_DATA_BYTES_PER_FRAME; i++)
			pucData[i] = (uint8_t)(rand());

		/*	Initiate transmission to self	*/
		vHOS_RF_send(&xRF, xRF.ucSelfAddress, pucData, uiRF_DATA_BYTES_PER_FRAME, 0);

		/*	wait for transmission to be done	*/
		while (xRF.ucTxEmptyFalg != 1);

		/*	Compare received data with the sent one	*/
		ucMatch = 1;
		for (uint32_t i = 0; i < uiRF_DATA_BYTES_PER_FRAME; i++)
		{
			if (xRF.pucRxBuffer[i] != pucData[i])
				ucMatch = 0;
		}

		if (ucMatch == 1)
			uiSuccessCount++;

		uiTotalCount++;

		fSuccessRatio = (float)uiSuccessCount / (float)uiTotalCount;

		/*	random delay to test HW responsiveness after being idle	*/
		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(((uint32_t)rand())%2000));
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
}

/*******************************************************************************
 * HAL_OS objects initialization:
 ******************************************************************************/
void obj_init(void)
{
	xRF.ucTxPort = 0;
	xRF.ucTxPin = 0;

	xRF.ucRxPort = 0;
	xRF.ucRxPin = 1;

	xRF.ucSelfAddress = 0x30;

	vHOS_RF_init(&xRF);
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
