/*
 * Button.h
 *
 *  Created on: Jun 8, 2023
 *      Author: Ali Emad
 */

#ifndef HAL_OS_INC_BUTTON_BUTTON_H_
#define HAL_OS_INC_BUTTON_BUTTON_H_

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
 * 				1-	Button's level is invert of "ucPressedLevel". (i.e.: "ucCurrentState" = "RELEASED")
 * 				2-	Button's level is "ucPressedLevel". (i.e.: "ucCurrentState" = "PRE_PRESSED")
 * 				3-	Button's level is "ucPressedLevel". (i.e.: "ucCurrentState" = "PRESSED")
 */
typedef struct{
	uint8_t ucPortNumber;

	uint8_t ucPinNumber;

	void (*pfCallback)(void);

	uint8_t ucPressedLevel;

	uint8_t ucCurrentState;
}HOS_Button_t;

/*
 * Initializes button object.
 *
 * Notes:
 * 		-	"configHOS_BUTTON_EN" must be initially set in order for the driver to work.
 *
 *		-	user can't create buttons more than "configHOS_BUTTON_MAX_NUMBER_OF_BUTTONS".
 *
 *		-	"xButtonHandle" would be assigned by this function to a pointer of
 *			the just created button. If not needed, it is set to "NULL".
 */
void vHOS_Button_init(	uint8_t ucPortNumber,
						uint8_t ucPinNumber,
						void (*pfCallback)(void),
						uint8_t ucPressedLevel,
						HOS_Button_t* xButtonHandle	);

/*
 * Deletes button object.
 */
void vHOS_Button_delete(HOS_Button_t* xButtonHandle);

#endif /* HAL_OS_INC_BUTTON_BUTTON_H_ */
