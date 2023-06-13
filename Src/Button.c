/*
 * Button.c
 *
 *  Created on: Jun 8, 2023
 *      Author: Ali Emad
 */

/*	LIB	*/
#include <stdint.h>

/*	FreeRTOS	*/
#include "FreeRTOS.h"
#include "task.h"

/*	MCAL (Ported)	*/
#include "Port/Port_DIO.h"
#include "Port/Port_Print.h"
#include "Port/Port_Breakpoint.h"

/*	SELF	*/
#include "Inc/Button/Button.h"

#if configHOS_BUTTON_EN

/*******************************************************************************
 * Private configurations:
 ******************************************************************************/
#define uiBUTTON_STACK_SIZE		configMINIMAL_STACK_SIZE

/*******************************************************************************
 * Static objects:
 ******************************************************************************/
static xHOS_Button_t xButtonArr[configHOS_BUTTON_MAX_NUMBER_OF_BUTTONS];
static uint16_t usNumberOfUsedButtons = 0;

/*	Driver's stacks and task handle	*/
static StackType_t pxButtonStack[uiBUTTON_STACK_SIZE];
static StaticTask_t xButtonStaticTask;
static TaskHandle_t xButtonTaskHandle;

/*******************************************************************************
 * Helping functions/macros.
 ******************************************************************************/
#define RELEASED 		0
#define PRE_PRESSED		1
#define PRESSED			2

#define pxTOP_PTR	(xButtonArr + configHOS_BUTTON_MAX_NUMBER_OF_BUTTONS * sizeof(xHOS_Button_t))

/*******************************************************************************
 * RTOS Task code:
 ******************************************************************************/
/*
 * If "configHOS_BUTTON_EN" was enabled, a task that uses this function will be
 * created in the "xHOS_init()".
 *
 * For further information about how HAL_OS drivers are managed, refer to the
 * repository's "README.md"
 */
void vButton_manager(void* pvParams)
{
	uint8_t i;
	uint8_t pinLevel;

	while(1)
	{
		/*	loop on created buttons	*/
		for (i = 0; i < usNumberOfUsedButtons; i++)
		{
			/*	if button is disabled, skip it	*/
			if (xButtonArr[i].ucIsEnabled == 0)
				continue;

			/*	read digital level of button's DIO pin	*/
			pinLevel = ucPort_DIO_readPin(xButtonArr[i].ucPortNumber, xButtonArr[i].ucPinNumber);

			/*	if it is the pressed level	*/
			if (pinLevel == xButtonArr[i].ucPressedLevel)
			{
				/*	if button was previously in "PRESSED" state	*/
				if (xButtonArr[i].ucCurrentState == PRESSED)
				{	/*	Do nothing	*/	}
				/*	otherwise, if "PRE_PRESSED" or "RELEASED"	*/
				else
				{
					/*	increment number of pressed samples	*/
					xButtonArr[i].ucNumberOfPressedSamples++;
					/*	if equal to filtering number of samples	*/
					if (xButtonArr[i].ucNumberOfPressedSamples == xButtonArr[i].ucFilterN)
					{
						/*	transit to "PRESSED" state and execute callback	*/
						xButtonArr[i].ucCurrentState = PRESSED;
						xButtonArr[i].pfCallback();
					}
					/*	otherwise, button is in "PRE_PRESSED" state	*/
					xButtonArr[i].ucCurrentState = PRE_PRESSED;
				}
			}

			/*	otherwise, if it is the released level	*/
			else
			{
				xButtonArr[i].ucCurrentState = RELEASED;
				xButtonArr[i].ucNumberOfPressedSamples = 0;
			}
		}

		/*	Delay until next sample time	*/
		vTaskDelay(pdMS_TO_TICKS(configHOS_BUTTON_SAMPLING_INTERVAL_MS));
	}
}

/*
 * Initializes the task responsible for this  driver.
 * Notes:
 * 		-	This function is externed and called in "HAL_OS.c".
 */
BaseType_t xButton_initTask(void)
{
	xButtonTaskHandle = xTaskCreateStatic(	vButton_manager,
											"Button",
											uiBUTTON_STACK_SIZE,
											NULL,
											configNORMAL_TASK_PRIORITY,
											pxButtonStack,
											&xButtonStaticTask	);
	if (xButtonTaskHandle == NULL)
		return pdFAIL;

	else
		return pdPASS;
}

/*******************************************************************************
 * API functions:
 ******************************************************************************/
/*
 * See header file for info.
 */
xHOS_Button_t* pxHOS_Button_init(	uint8_t ucPortNumber,
									uint8_t ucPinNumber,
									void (*pfCallback)(void),
									uint8_t ucPressedLevel,
									uint8_t ucFilterN	)
{
	/*	check (assert) if there's enough memory to add new button	*/
	configASSERT(usNumberOfUsedButtons < configHOS_BUTTON_MAX_NUMBER_OF_BUTTONS);

	/*	initialize DIO pin as an input, pulled with invert of "ucPressedLevel"	*/
	uint8_t ucPull = ucPressedLevel ? 2 : 1;
	vPort_DIO_initPinInput(ucPortNumber, ucPinNumber, ucPull);

	/*	store button data in "xButtonArr"	*/
	xHOS_Button_t* pxHandle = &xButtonArr[usNumberOfUsedButtons];
	pxHandle->ucPortNumber = ucPortNumber;
	pxHandle->ucPinNumber = ucPinNumber;
	pxHandle->pfCallback = pfCallback;
	pxHandle->ucPressedLevel = ucPressedLevel;
	pxHandle->ucFilterN = ucFilterN;
	pxHandle->ucCurrentState = RELEASED;
	pxHandle->ucNumberOfPressedSamples = 0;
	pxHandle->ucIsEnabled = 1;

	/*	Increment buttons counter	*/
	usNumberOfUsedButtons++;

	/*	return pointer to the new handle	*/
	return pxHandle;
}

/*
 * See header file for info.
 */
void vHOS_Button_Enable(xHOS_Button_t* pxButtonHandle)
{
	/*	check pointer first	*/
	configASSERT(xButtonArr <= pxButtonHandle && pxButtonHandle < pxTOP_PTR);

	/*	Enable	*/
	xButtonArr[usNumberOfUsedButtons].ucIsEnabled = 1;
}

/*
 * See header file for info.
 */
void vHOS_Button_Disable(xHOS_Button_t* pxButtonHandle)
{
	/*	check pointer first	*/
	configASSERT(xButtonArr <= pxButtonHandle && pxButtonHandle < xButtonArr + configHOS_BUTTON_MAX_NUMBER_OF_BUTTONS * sizeof(xHOS_Button_t));

	/*	Disable	*/
	xButtonArr[usNumberOfUsedButtons].ucIsEnabled = 0;
}

#endif	/*	configHOS_BUTTON_EN	*/
