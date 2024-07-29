/*
 * Port_DIO.c
 *
 *  Created on: Jun 9, 2023
 *      Author: Ali Emad
 */

/*	Target checking	*/
#include "MCAL_Port/Port_Target.h"
#ifdef MCAL_PORT_TARGET_STM32F401RCT6


#include "stm32f401xc.h"
#include "stm32f4xx_hal.h"

#include "HAL/IOExtend/OExtendShiftRegister.h"

GPIO_TypeDef* const pxPortDioPortArr[] = {GPIOA, GPIOB, GPIOC};

xHOS_OExtendShiftRegister_t pxPortDioOutputExtendedPortArr[2];

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
	/*	If port is of extended ports, return	*/
	if (ucPortNumber > 2)
		return;

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

void vPort_DIO_initExtendedOutputPorts(void)
{
	pxPortDioOutputExtendedPortArr[0].ucSpiUnitNumber = 0;
	pxPortDioOutputExtendedPortArr[0].ucRegisterSizeInBytes = 2;
	pxPortDioOutputExtendedPortArr[0].ucLatchPort = 0;
	pxPortDioOutputExtendedPortArr[0].ucLatchPin = 6;
	vHOS_OExtendShiftRegister_init(&pxPortDioOutputExtendedPortArr[0]);

	pxPortDioOutputExtendedPortArr[1].ucSpiUnitNumber = 255;
	pxPortDioOutputExtendedPortArr[1].ucRegisterSizeInBytes = 1;
	pxPortDioOutputExtendedPortArr[1].ucSckPort = 1;
	pxPortDioOutputExtendedPortArr[1].ucSckPin = 8;
	pxPortDioOutputExtendedPortArr[1].ucDataPort = 1;
	pxPortDioOutputExtendedPortArr[1].ucDataPin = 7;
	pxPortDioOutputExtendedPortArr[1].ucLatchPort = 1;
	pxPortDioOutputExtendedPortArr[1].ucLatchPin = 6;
	vHOS_OExtendShiftRegister_init(&pxPortDioOutputExtendedPortArr[1]);
}


#endif /* Target checking */
