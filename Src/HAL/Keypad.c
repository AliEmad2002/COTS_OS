/*
 * Keypad.c
 *
 *  Created on: Feb 4, 2024
 *      Author: Ali Emad
 */

/*	LIB	*/
#include <stdint.h>

/*	RTOS	*/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "RTOS_PRI_Config.h"

/*	MCAL	*/
#include "MCAL_Port/Port_DIO.h"

/*	SELF	*/
#include "HAL/Keypad/Keypad.h"


/*******************************************************************************
 * RTOS task:
 ******************************************************************************/
volatile uint32_t uiNPresses = 0;
static void vTask(void* pvParams)
{
	xHOS_Keypad_t* pxHandle = (xHOS_Keypad_t*)pvParams;

	while(1)
	{
		/*	Execute one scan	*/

		/*	For every column pin	*/
		for (uint8_t ucCol = 0; ucCol < pxHandle->ucNCols; ucCol++)
		{
			/*	Set the i-th column pin	*/
			vPORT_DIO_WRITE_PIN(	pxHandle->pucColPortArr[ucCol],
									pxHandle->pucColPinArr[ucCol],
									1	);

			/*
			 * Settling delay (Wait for resetting previous column pin, and setting
			 * new one to settle)
			 */
			vTaskDelay(pdMS_TO_TICKS(pxHandle->uiMsSettlingDelay));

			/*	For every row pin	*/
			for (uint8_t ucRow = 0; ucRow < pxHandle->ucNRows; ucRow++)
			{
				/*	Read pin state	*/
				uint8_t ucRowPinState = ucPORT_DIO_READ_PIN(
						pxHandle->pucRowPortArr[ucRow],
						pxHandle->pucRowPinArr[ucRow]	);

				/*	If row pin is high, then pin[row][col] is now pressed.	*/
				if (ucRowPinState == 1)
				{
					pxHandle->cLastPressedKey =
							pxHandle->pcButtons[ucRow * pxHandle->ucNCols + ucCol];

					/*	Give new press semaphore	*/
					xSemaphoreGive(pxHandle->xNewPressedKeySemphr);

					uiNPresses++;

					/*	Debouncing delay	*/
					vTaskDelay(pdMS_TO_TICKS(pxHandle->uiMsDebouncingDelay));
				}
			}

			/*	Reset the i-th column pin	*/
			vPORT_DIO_WRITE_PIN(	pxHandle->pucColPortArr[ucCol],
									pxHandle->pucColPinArr[ucCol],
									0	);
		}
	}
}

/*******************************************************************************
 * API functions:
 ******************************************************************************/
/*
 * See header for info.
 */
void vHOS_Keypad_init(xHOS_Keypad_t* pxHandle)
{
	/*	Initialize all row pins as pulled-down inputs	*/
	for (uint8_t i = 0; i < pxHandle->ucNRows; i++)
	{
		vPort_DIO_initPinInput(	pxHandle->pucRowPortArr[i],
								pxHandle->pucRowPinArr[i],
								2	);
	}

	/*	Initialize all col pins as outputs (initially low)	*/
	for (uint8_t i = 0; i < pxHandle->ucNCols; i++)
	{
		vPort_DIO_initPinOutput(	pxHandle->pucColPortArr[i],
									pxHandle->pucColPinArr[i]	);

		vPORT_DIO_WRITE_PIN(	pxHandle->pucColPortArr[i],
								pxHandle->pucColPinArr[i],
								0	);
	}

	/*
	 * "cLastPressedKey" is initially '\0'. Indicating that no keys have been
	 * pressed.
	 */
	pxHandle->cLastPressedKey = '\0';

	/*	Initialize new press semaphore	*/
	pxHandle->xNewPressedKeySemphr =
			xSemaphoreCreateBinaryStatic(&pxHandle->xNewPressedKeySemphrStatic);

	xSemaphoreTake(pxHandle->xNewPressedKeySemphr, 0);

	/*	Initialize handle's task	*/
	pxHandle->xTask = xTaskCreateStatic(
		vTask,
		"keypad",
		configMINIMAL_STACK_SIZE,
		(void*)pxHandle,
		configHOS_SOFT_REAL_TIME_TASK_PRI,
		pxHandle->puxTaskStack,
		&pxHandle->xTaskStatic);
}

uint8_t ucHOS_Keypad_waitKey(	xHOS_Keypad_t* pxHandle,
								char* pcButtonVal,
								TickType_t xTimeout	)
{
	uint8_t ucState =
			xSemaphoreTake(pxHandle->xNewPressedKeySemphr, xTimeout);

	if (!ucState)
		return 0;

	*pcButtonVal = pxHandle->cLastPressedKey;

	return 1;
}


























