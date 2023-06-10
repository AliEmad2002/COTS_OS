/*
 * Button.h
 *
 *  Created on: Jun 8, 2023
 *      Author: Ali Emad
 */

#ifndef HAL_OS_INC_BUTTON_BUTTON_H_
#define HAL_OS_INC_BUTTON_BUTTON_H_

#include "Inc/Button/ButtonConfig.h"

/*
 * "Button" structure.
 *
 * Notes:
 * 		-	"pfCallback" function must be very short in terms of execution time.
 * 			if needed to be longer, it may be implemented as a FreeRTOS task
 * 			and just get controlled by buttons callback.
 *
 * 		-	"ucPressedLevel": 0==> low level, 1==> high level.
 *
 * 		-	Presses are filtered using the simple pressed/pre-pressed method.
 * 			That is, callback will be executed only after these three states occur
 * 			in sequence:
 * 				1-	Button's level is invert of "ucPressedLevel".
 * 					(i.e.: "ucCurrentState" = "RELEASED")
 * 				2-	Button's level is "ucPressedLevel". for "ucFilterN-1" consecutive
 * 					samples. (i.e.: "ucCurrentState" = "PRE_PRESSED")
 * 				3-	Button's level is "ucPressedLevel". for the "ucFilterN"th sample
 * 					in row. (i.e.: "ucCurrentState" = "PRESSED")
 */
typedef struct{
	/*	configuration parameters	*/
	uint8_t ucPortNumber;

	uint8_t ucPinNumber;

	void (*pfCallback)(void);

	uint8_t ucPressedLevel;

	uint8_t ucFilterN;

	/*	runtime changing parameters	*/
	uint8_t ucCurrentState;

	uint8_t ucNumberOfPressedSamples;

	uint8_t ucIsEnabled;
}xHOS_Button_t;

/*
 * Initializes button object.
 *
 * Notes:
 * 		-	"configHOS_BUTTON_EN" must be initially set in order for the driver to work.
 *
 *		-	user can't create buttons more than "configHOS_BUTTON_MAX_NUMBER_OF_BUTTONS".
 *
 *		-	returns pointer to the created handle.
 */
xHOS_Button_t* pxHOS_Button_init(	uint8_t ucPortNumber,
									uint8_t ucPinNumber,
									void (*pfCallback)(void),
									uint8_t ucPressedLevel,
									uint8_t ucFilterN	);

/*
 * Enables button.
 *
 * Notes:
 * 		-	Buttons are initially enabled on creation.
 */
void vHOS_Button_Enable(xHOS_Button_t* pxButtonHandle);

/*
 * Disables button. (Presses are ignored)
 *
 * Notes:
 * 		-	Buttons are initially enabled on creation.
 */
void vHOS_Button_Disable(xHOS_Button_t* pxButtonHandle);

/*
 * Reads button.
 *
 * Notes:
 * 		-	Returns 1 if button state is "PRESSED", 0 otherwise.
 */
uint8_t ucHOS_Button_Read(xHOS_Button_t* pxButtonHandle);

#endif /* HAL_OS_INC_BUTTON_BUTTON_H_ */
