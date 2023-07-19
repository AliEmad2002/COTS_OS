/*
 * Port_AFIO.h
 *
 *  Created on: Jun 13, 2023
 *      Author: Ali Emad
 */

#ifndef HAL_OS_PORT_PORT_AFIO_H_
#define HAL_OS_PORT_PORT_AFIO_H_

/*
 * In most of ARM-based MCUs, peripherals are not directly mapped to a constant
 * set ofIO pins. Instead, another peripheral (Alternate Function IO) configures
 * the mapping.
 *
 * If the ported target does not have one, mapping functions are left empty.
 */

#include "stm32f1xx_hal.h"

static inline void vPort_AFIO_mapSpi(uint8_t ucUnitNumber, uint8_t ucMapNumber)
{
	if (ucUnitNumber == 1)	/*	SPI2 in STM32F103 has one map only	*/
		return;

	if (ucMapNumber == 0)
		__HAL_AFIO_REMAP_SPI1_DISABLE();
	else
		__HAL_AFIO_REMAP_SPI1_ENABLE();
}

static inline void vPort_AFIO_mapI2C(uint8_t ucUnitNumber, uint8_t ucMapNumber)
{
	/*	I2C in STM32F103 has one map only	*/
}



#endif /* HAL_OS_PORT_PORT_AFIO_H_ */
