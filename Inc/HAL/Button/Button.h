/*
 * Button.h
 *
 *  Created on: Jun 8, 2023
 *      Author: Ali Emad
 */

#ifndef HAL_OS_INC_BUTTON_BUTTON_H_
#define HAL_OS_INC_BUTTON_BUTTON_H_

#include "FreeRTOS.h"
#include "task.h"

#define uiBUTTON_STACK_SIZE			100

/*
 * "Button" structure.
 *
 * Notes:
 * 		-	"pfCallback" function must be very short in terms of execution time
 * 			and small in terms of stack size. if needed to be longer/larger, it
 * 			may be implemented as a FreeRTOS task and just get controlled by
 * 			buttons callback.
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
	/*		PRIVATE		*/
	uint8_t ucCurrentState;
	uint8_t ucNumberOfPressedSamples;

	StackType_t puxTaskStack[uiBUTTON_STACK_SIZE];
	StaticTask_t xTaskStatic;
	TaskHandle_t xTask;

	/*		PUBLIC		*/
	uint8_t ucPortNumber;
	uint8_t ucPinNumber;

	void (*pfCallback)(void*);

	uint8_t ucPressedLevel;

	uint8_t ucFilterN;

	uint32_t uiSamplePeriodMs;
}xHOS_Button_t;

/*
 * Initializes button object.
 * Notes:
 * 		-	All public variables of the passed handle must be initialized to valid
 * 			values first.
 *
 * 		-	Must be called before scheduler start.
 */
void vHOS_Button_init(xHOS_Button_t* pxHandle);

/*
 * Enables button.
 *
 * Notes:
 * 		-	Buttons are initially enabled on creation.
 *
 * 		-	This function is inline.
 */
void vHOS_Button_Enable(xHOS_Button_t* pxHandle);

/*
 * Disables button. (Presses are ignored)
 *
 * Notes:
 * 		-	Buttons are initially enabled on creation.
 *
 * 		-	This function is inline.
 */
void vHOS_Button_Disable(xHOS_Button_t* pxHandle);

/*
 * Reads button.
 *
 * Notes:
 * 		-	Returns 1 if button state is "PRESSED", 0 otherwise.
 *
 * 		-	This function is inline.
 */
uint8_t ucHOS_Button_Read(xHOS_Button_t* pxHandle);

#endif /* HAL_OS_INC_BUTTON_BUTTON_H_ */
