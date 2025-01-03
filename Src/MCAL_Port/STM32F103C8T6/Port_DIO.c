/*
 * Port_DIO.c
 *
 *  Created on: Jun 9, 2023
 *      Author: Ali Emad
 */

/*	Target checking	*/
#include "MCAL_Port/Port_Target.h"
#ifdef MCAL_PORT_TARGET_STM32F103C8T6


#include "stm32f103xb.h"
#include "stm32f1xx_hal.h"

GPIO_TypeDef* const pxPortDioPortArr[] = {GPIOA, GPIOB, GPIOC};


void vPort_DIO_initPinInput(uint8_t ucPortNumber, uint8_t ucPinNumber, uint8_t ucPull)
{
	GPIO_InitTypeDef conf = {
		.Pin = 1ul << ucPinNumber,
		.Mode = GPIO_MODE_INPUT,
		.Pull = ucPull,
		.Speed = GPIO_SPEED_FREQ_HIGH
	};

	HAL_GPIO_Init(pxPortDioPortArr[ucPortNumber], &conf);
}

void vPort_DIO_initPinOutput(uint8_t ucPortNumber, uint8_t ucPinNumber)
{
	GPIO_InitTypeDef conf = {
		.Pin = 1ul << ucPinNumber,
		.Mode = GPIO_MODE_OUTPUT_PP,
		.Pull = GPIO_NOPULL,
		.Speed = GPIO_SPEED_FREQ_HIGH
	};

	HAL_GPIO_Init(pxPortDioPortArr[ucPortNumber], &conf);
}

void vPort_DIO_initPinOutputOpenDrain(uint8_t ucPortNumber, uint8_t ucPinNumber)
{
	/*	If port is of extended ports, return	*/
	if (ucPortNumber >2)
		return;

	GPIO_InitTypeDef conf = {
		.Pin = 1ul << ucPinNumber,
		.Mode = GPIO_MODE_OUTPUT_OD,
		.Pull = GPIO_NOPULL,
		.Speed = GPIO_SPEED_FREQ_HIGH
	};

	HAL_GPIO_Init(pxPortDioPortArr[ucPortNumber], &conf);
}

void vPort_DIO_initPinOutputOpenDrainPU(uint8_t ucPortNumber, uint8_t ucPinNumber)
{
	/*	If port is of extended ports, return	*/
	if (ucPortNumber >2)
		return;

	GPIO_InitTypeDef conf = {
		.Pin = 1ul << ucPinNumber,
		.Mode = GPIO_MODE_OUTPUT_OD,
		.Pull = GPIO_PULLUP,
		.Speed = GPIO_SPEED_FREQ_HIGH
	};

	HAL_GPIO_Init(pxPortDioPortArr[ucPortNumber], &conf);
}

#endif /* Target checking */
