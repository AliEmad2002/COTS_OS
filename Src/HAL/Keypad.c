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

/*	MCAL	*/
#include "MCAL_Port/Port_DIO.h"

/*	SELF	*/
#include "HAL/Keypad/Keypad.h"


/*******************************************************************************
 * RTOS task:
 ******************************************************************************/
static void vTask(void* pvParams)
{
	xHOS_Keypad_t* pxHandle = (xHOS_Keypad_t*)pvParams;

	TickType_t xLastPressTime = 0;

	while(1)
	{
		/*	Execute one scan	*/
		pxHandle->ucNPressedKeys = 0;

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
					/*	If there's still space in "ucNPressedKeys" array	*/
					if (pxHandle->ucNPressedKeys < ucCONF_KEYPAD_MAX_NUMBER_OF_SIMULTANEOUSLY_PRESSED_KEYS)
					{
						/*	Add button value to the "ucNPressedKeys" array	*/
						pxHandle->pcPressedKeys[pxHandle->ucNPressedKeys++] =
								pxHandle->pcButtons[ucRow][ucCol]	;
					}

					/*
					 * Otherwise, there's no need to complete this scan, as no
					 * new presses can be accepted.
					 */
					else
					{
						/*	TODO	*/
					}
				}
			}

			/*	Reset the i-th column pin	*/
			vPORT_DIO_WRITE_PIN(	pxHandle->pucColPortArr[ucCol],
									pxHandle->pucColPinArr[ucCol],
									0	);
		}


	}
}






























