/*
 * Port_DIO.h
 *
 *  Created on: Jun 8, 2023
 *      Author: Ali Emad
 */

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
extern GPIO_TypeDef* const portArr[];

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
static inline void vHOS_DIO_initPinInput(uint8_t ucPortNumber, uint8_t ucPinNumber, uint8_t ucPull)
{
	GPIO_InitTypeDef conf = {
		.Pin = 1 << ucPinNumber,
		.Mode = GPIO_MODE_INPUT,
		.Pull = ucPull,
		.Speed = GPIO_SPEED_FREQ_HIGH
	};

	HAL_GPIO_Init(portArr[ucPortNumber], &conf);
}

/*
 * Initializes pin as digital output.
 *
 * Notes:
 * 		-	DIO clock (if controlled) must be initially enabled.
 */
static inline void vHOS_DIO_initPinOutput(uint8_t ucPortNumber, uint8_t ucPinNumber)
{
	GPIO_InitTypeDef conf = {
		.Pin = 1 << ucPinNumber,
		.Mode = GPIO_MODE_OUTPUT_PP,
		.Pull = GPIO_NOPULL,
		.Speed = GPIO_SPEED_FREQ_HIGH
	};

	HAL_GPIO_Init(portArr[ucPortNumber], &conf);
}

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
static inline uint8_t ucHOS_DIO_readPin(uint8_t ucPortNumber, uint8_t ucPinNumber)
{
	return HAL_GPIO_ReadPin(portArr[ucPortNumber], 1 << ucPinNumber);
}

/*
 * Writes pin output value
 *
 * Notes:
 * 		-	Pin must be initially initialized as digital output.
 *
 *		-	"state": 0 for low level, 1 for high level.
 *
 * 		-	DIO clock (if controlled) must be initially enabled.
 */
static inline void vHOS_DIO_writePin(uint8_t ucPortNumber, uint8_t ucPinNumber, uint8_t ucState)
{
	HAL_GPIO_WritePin(portArr[ucPortNumber], 1 << ucPinNumber, ucState);
}

/*
 * Toggles pin output value
 *
 * Notes:
 * 		-	Pin must be initially initialized as digital output.
 *
 * 		-	DIO clock (if controlled) must be initially enabled.
 */
static inline void vHOS_DIO_togglePin(uint8_t ucPortNumber, uint8_t ucPinNumber)
{
	HAL_GPIO_TogglePin(portArr[ucPortNumber], 1 << ucPinNumber);
}

#endif /* HAL_OS_PORT_PORT_DIO_H_ */
