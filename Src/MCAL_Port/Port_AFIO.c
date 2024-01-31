/*
 * Port_AFIO.c
 *
 *  Created on: Sep 25, 2023
 *      Author: Ali Emad
 */


#include "stdint.h"

#include "MCAL_Port/Port_AFIO.h"

void vPort_AFIO_mapSpi(uint8_t ucUnitNumber, uint8_t ucMapNumber)
{
	if (ucUnitNumber == 1)	/*	SPI2 in STM32F103 has one map only	*/
		return;

	if (ucMapNumber == 0)
		__HAL_AFIO_REMAP_SPI1_DISABLE();
	else
		__HAL_AFIO_REMAP_SPI1_ENABLE();
}

void vPort_AFIO_mapI2C(uint8_t ucUnitNumber, uint8_t ucMapNumber)
{
	/*	I2C in STM32F103 has one map only	*/
}

void vPort_AFIO_mapUart(uint8_t ucUnitNumber, uint8_t ucMapNumber)
{
	if (ucUnitNumber == 0)	/*	USART2 & USART3 in STM32F103 has one map only	*/
	{
		if (ucMapNumber == 0)
			__HAL_AFIO_REMAP_USART1_DISABLE();
		else
			__HAL_AFIO_REMAP_USART1_ENABLE();
	}
}

