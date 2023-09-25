/*
 * Button.c
 *
 *  Created on: Jun 8, 2023
 *      Author: Ali Emad
 */

/*	LIB	*/
#include <stdint.h>
#include <stdio.h>

/*	FreeRTOS	*/
#include "FreeRTOS.h"
#include "task.h"

/*	MCAL (Ported)	*/
#include "MCAL_Port/Port_DIO.h"

/*	HAL-OS	*/
#include "RTOS_PRI_Config.h"

/*	SELF	*/
#include "HAL/Button/Button.h"

/*******************************************************************************
 * Helping functions/macros.
 ******************************************************************************/
#define RELEASED 		0
#define PRE_PRESSED		1
#define PRESSED			2

/*******************************************************************************
 * RTOS Task code:
 ******************************************************************************/
static void vTask(void* pvParams)
{
	xHOS_Button_t* pxHandle = (xHOS_Button_t*)pvParams;

	uint8_t pinLevel;
	TickType_t xLastWakeTime = xTaskGetTickCount();
	while(1)
	{
		/*	read digital level of button's DIO pin	*/
		pinLevel = ucPORT_DIO_READ_PIN(pxHandle->ucPortNumber, pxHandle->ucPinNumber);

		/*	if it is the pressed level	*/
		if (pinLevel == pxHandle->ucPressedLevel)
		{
			/*	if button was previously in "PRESSED" state	*/
			if (pxHandle->ucCurrentState == PRESSED)
			{	/*	Do nothing	*/	}
			/*	otherwise, if "PRE_PRESSED" or "RELEASED"	*/
			else
			{
				/*	increment number of pressed samples	*/
				pxHandle->ucNumberOfPressedSamples++;
				/*	if equal to filtering number of samples	*/
				if (pxHandle->ucNumberOfPressedSamples == pxHandle->ucFilterN)
				{
					/*	transit to "PRESSED" state and execute callback	*/
					pxHandle->ucCurrentState = PRESSED;
					pxHandle->pfCallback();
				}
				/*	otherwise, button is in "PRE_PRESSED" state	*/
				pxHandle->ucCurrentState = PRE_PRESSED;
			}
		}

		/*	otherwise, if it is the released level	*/
		else
		{
			pxHandle->ucCurrentState = RELEASED;
			pxHandle->ucNumberOfPressedSamples = 0;
		}

		/*	Task is blocked until next sample time	*/
		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(pxHandle->uiSamplePeriodMs));
	}
}

/*******************************************************************************
 * API functions:
 ******************************************************************************/
/*
 * See header file for info.
 */
void vHOS_Button_init(xHOS_Button_t* pxHandle)
{
	/*	initialize DIO pin as an input, pulled with invert of "ucPressedLevel"	*/
	uint8_t ucPull = pxHandle->ucPressedLevel ? 2 : 1;
	vPort_DIO_initPinInput(pxHandle->ucPortNumber, pxHandle->ucPinNumber, ucPull);

	/*	initialize private parameters	*/
	pxHandle->ucCurrentState = RELEASED;
	pxHandle->ucNumberOfPressedSamples = 0;

	/*	create task	*/
	static uint8_t ucCreatedObjectsCount = 0;
	char pcTaskName[configMAX_TASK_NAME_LEN];
	sprintf(pcTaskName, "Button%d", ucCreatedObjectsCount++);

	pxHandle->xTask = xTaskCreateStatic(	vTask,
											pcTaskName,
											configMINIMAL_STACK_SIZE,
											(void*)pxHandle,
											configHOS_SOFT_REAL_TIME_TASK_PRI,
											pxHandle->puxTaskStack,
											&pxHandle->xTaskStatic	);
}

/*
 * See header file for info.
 */
__attribute__((always_inline)) inline
void vHOS_Button_Enable(xHOS_Button_t* pxHandle)
{
	vTaskResume(pxHandle->xTask);
}

/*
 * See header file for info.
 */
__attribute__((always_inline)) inline
void vHOS_Button_Disable(xHOS_Button_t* pxHandle)
{
	vTaskSuspend(pxHandle->xTask);
}

/*
 * See header file for info.
 */
__attribute__((always_inline)) inline
uint8_t ucHOS_Button_Read(xHOS_Button_t* pxHandle)
{
	return (pxHandle->ucCurrentState == PRESSED) ? 1 : 0;
}






