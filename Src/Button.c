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

static xHOS_Button_t xButtonArr[configHOS_BUTTON_MAX_NUMBER_OF_BUTTONS];
static uint16_t usNumberOfUsedButtons = 0;

#define RELEASED 		0
#define PRE_PRESSED		1
#define PRESSED			2

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
			pinLevel = ucHOS_DIO_readPin(xButtonArr[i].ucPortNumber, xButtonArr[i].ucPinNumber);

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
 * See header file for info.
 */
void vHOS_Button_init(	uint8_t ucPortNumber,
						uint8_t ucPinNumber,
						void (*pfCallback)(void),
						uint8_t ucPressedLevel,
						uint8_t ucFilterN,
						xHOS_Button_t* pxButtonHandle	)
{
	/*	check (assert) if there's enough memory to add new button	*/
	configASSERT(usNumberOfUsedButtons < configHOS_BUTTON_MAX_NUMBER_OF_BUTTONS);
//	if (usNumberOfUsedButtons == configHOS_BUTTON_MAX_NUMBER_OF_BUTTONS)
//	{
//		vHOS_Print("Number of maximum possible buttons reached!\n");
//		vHOS_Breakpoint();
//		while(1);
//	}

	/*	initialize DIO pin as an input, pulled with invert of "ucPressedLevel"	*/
	uint8_t ucPull = ucPressedLevel ? 2 : 1;
	vHOS_DIO_initPinInput(ucPortNumber, ucPinNumber, ucPull);

	/*	Assign pointer to button handle	*/
	if (pxButtonHandle != NULL)
		pxButtonHandle = &xButtonArr[usNumberOfUsedButtons];

	/*	store button data in "xButtonArr"	*/
	xButtonArr[usNumberOfUsedButtons].ucPortNumber = ucPortNumber;
	xButtonArr[usNumberOfUsedButtons].ucPinNumber = ucPinNumber;
	xButtonArr[usNumberOfUsedButtons].pfCallback = pfCallback;
	xButtonArr[usNumberOfUsedButtons].ucPressedLevel = ucPressedLevel;
	xButtonArr[usNumberOfUsedButtons].ucFilterN = ucFilterN;
	xButtonArr[usNumberOfUsedButtons].ucCurrentState = RELEASED;
	xButtonArr[usNumberOfUsedButtons].ucNumberOfPressedSamples = 0;
	xButtonArr[usNumberOfUsedButtons].ucIsEnabled = 1;

	/*	Increment buttons counter	*/
	usNumberOfUsedButtons++;
}

/*
 * See header file for info.
 */
void vHOS_Button_Enable(xHOS_Button_t* pxButtonHandle)
{
	/*	check pointer first	*/
	configASSERT(xButtonArr <= pxButtonHandle && pxButtonHandle < xButtonArr + configHOS_BUTTON_MAX_NUMBER_OF_BUTTONS * sizeof(xHOS_Button_t));

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
