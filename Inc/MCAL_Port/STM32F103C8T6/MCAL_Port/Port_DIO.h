/*
 * Port_DIO.h
 *
 *  Created on: Jun 8, 2023
 *      Author: Ali Emad
 */

/*	Target checking	*/
#include "MCAL_Port/Port_Target.h"
#ifdef MCAL_PORT_TARGET_STM32F103C8T6


#ifndef HAL_OS_PORT_PORT_DIO_H_
#define HAL_OS_PORT_PORT_DIO_H_

/*
 * DIO driver of target MCU is included here, and its proper functions are called
 * inside the following defined wrapper functions.
 *
 * If such a function is not provided by the available driver, user may write it
 * inside the wrapper function (if too long to be inlined, remove the "inline"
 * identifier).
 */
#include "stm32f103xb.h"
#include "stm32f1xx_hal.h"

/*
 * Driver-long needed values.
 *
 * Notes:
 * 		-	Values which are not used in the current target can be deleted.
 *
 * 		-	If additional values are needed, they can be added here.
 *
 * 		-	Remember that static variables/functions are created each time the
 * 			file gets included, and non-static definitions in header cause "redefinition error".
 * 			Therefore, variables (if any) and non-static nor inline functions better
 * 			be defined in "Port_DIO.c" and externed here.
 * 			On the other hand, static and inline functions are normally defined
 * 			in a header.
 */
extern GPIO_TypeDef* const pxPortDioPortArr[];

/*
 * Initializes pin as digital input.
 *
 * Notes:
 * 		-	"ucPull" has three states:
 * 			0 ==> no pull
 * 			1 ==> pull up
 * 			2 ==> pull down
 *
 * 		-	DIO clock (if controlled) must be initially enabled.
 */
void vPort_DIO_initPinInput(uint8_t ucPortNumber, uint8_t ucPinNumber, uint8_t ucPull);

/*
 * Initializes pin as digital output.
 *
 * Notes:
 * 		-	DIO clock (if controlled) must be initially enabled.
 */
void vPort_DIO_initPinOutput(uint8_t ucPortNumber, uint8_t ucPinNumber);

/*
 * Initializes pin as open drain digital output.
 *
 * Notes:
 * 		-	Not all MCUs, (or all pins in some MCUs) support this feature. Thus,
 * 			a proper assertion should be implemented in the ".c" file of this driver.
 *
 * 		-	DIO clock (if controlled) must be initially enabled.
 */
void vPort_DIO_initPinOutputOpenDrain(uint8_t ucPortNumber, uint8_t ucPinNumber);

/*
 * Initializes pin as open drain digital output with pull-up.
 *
 * Notes:
 * 		-	Not all MCUs, (or all pins in some MCUs) support this feature. Thus,
 * 			a proper assertion should be implemented in the ".c" file of this driver.
 *
 * 		-	DIO clock (if controlled) must be initially enabled.
 */
void vPort_DIO_initPinOutputOpenDrainPU(uint8_t ucPortNumber, uint8_t ucPinNumber);

/*
 * Reads pin input value
 *
 * Notes:
 * 		-	Pin must be initially initialized as digital input.
 *
 *		-	Returns 0 if pin level is low, 1 if high.
 *
 * 		-	DIO clock (if controlled) must be initially enabled.
 */
#define ucPORT_DIO_READ_PIN(ucPortNumber, ucPinNumber)	\
	(	HAL_GPIO_ReadPin(pxPortDioPortArr[(ucPortNumber)], 1 << (ucPinNumber))	)

/*
 * Writes pin output value
 *
 * Notes:
 * 		-	Pin must be initially initialized as digital output.
 *
 *		-	"ucLevel": 0 for low level, 1 for high level.
 *
 * 		-	DIO clock (if controlled) must be initially enabled.
 */
#define vPORT_DIO_WRITE_PIN(ucPortNumber, ucPinNumber, ucLevel)	\
	(	HAL_GPIO_WritePin(pxPortDioPortArr[(ucPortNumber)], 1 << (ucPinNumber), (ucLevel))	)

/*
 * Toggles pin output value
 *
 * Notes:
 * 		-	Pin must be initially initialized as digital output.
 *
 * 		-	DIO clock (if controlled) must be initially enabled.
 */
#define vPORT_DIO_TOGGLE_PIN(ucPortNumber, ucPinNumber)	\
	(	HAL_GPIO_TogglePin(pxPortDioPortArr[(ucPortNumber)], 1 << (ucPinNumber))	)

/*
 * Reads complete port.
 *
 * Notes:
 * 		-	This function is very useful when there's need for synchronized reading
 * 			of multiple pins.
 *
 * 		-	Return value has level of pin_0 in bit_0.. and so on.
 */
#define uiPORT_DIO_READ_PORT(ucPortNumber)	\
	(	pxPortDioPortArr[(ucPortNumber)]->IDR	)

#endif /* HAL_OS_PORT_PORT_DIO_H_ */


#endif /* Target checking */
