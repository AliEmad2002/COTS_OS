/*
 * Keypad.h
 *
 *  Created on: Feb 4, 2024
 *      Author: Ali Emad
 */

#ifndef COTS_OS_INC_HAL_KEYPAD_KEYPAD_H_
#define COTS_OS_INC_HAL_KEYPAD_KEYPAD_H_

#include "HAL/Keypad/Keypad_Conf.h"

typedef struct{
	/*		PUBLIC		*/
	/*	Size of the keypad handle	*/
	uint8_t ucNRows;
	uint8_t ucNCols;

	/*	Pins of keypad handle	*/
	uint8_t* pucRowPortArr;
	uint8_t* pucRowPinArr;
	uint8_t* pucColPortArr;
	uint8_t* pucColPinArr;

	/*
	 * 2-D array representing values of keypad buttons.
	 *
	 * Example for 4x3 keypad:
	 * 		pcButtons = {
	 * 			'1', '2', '3',
	 * 			'4', '5', '6',
	 * 			'7', '8', '9',
	 * 			'*', '0', '#'
	 * 		};
	 */
	char* pcButtons;

	/*
	 * Debouncing delay in ms.
	 */
	uint32_t uiMsDebouncingDelay;

	/*
	 * Settling delay.
	 * (Minimum time required until signal on one the pins settles)
	 */
	uint32_t uiMsSettlingDelay;

	/*		PRIVATE		*/
	char cLastPressedKey;

	SemaphoreHandle_t xNewPressedKeySemphr;
	StaticSemaphore_t xNewPressedKeySemphrStatic;

	TaskHandle_t xTask;
	StaticTask_t xTaskStatic;
	StackType_t puxTaskStack[configMINIMAL_STACK_SIZE];
}xHOS_Keypad_t;

/*
 * Initializes keypad object.
 *
 * Notes:
 * 		-	All public variables of keypad handle must be initialized first.
 */
void vHOS_Keypad_init(xHOS_Keypad_t* pxHandle);

/*
 * Blocks until a key is pressed.
 *
 * Notes:
 * 		-	If key is pressed within the given timeout, functions returns 1, and
 * 			value of the pressed button is written in "pcButtonVal".
 *
 * 		-	Otherwise, if timeout passes while no buttons were pressed, function
 * 			returns 0.
 */
uint8_t ucHOS_Keypad_waitKey(	xHOS_Keypad_t* pxHandle,
								char* pcButtonVal,
								TickType_t xTimeout	);


















#endif /* COTS_OS_INC_HAL_KEYPAD_KEYPAD_H_ */
