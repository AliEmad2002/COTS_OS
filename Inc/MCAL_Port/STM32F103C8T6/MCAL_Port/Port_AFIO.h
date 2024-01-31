/*
 * Port_AFIO.h
 *
 *  Created on: Jun 13, 2023
 *      Author: Ali Emad
 */

/*	Target checking	*/
#include "MCAL_Port/Port_Target.h"
#ifdef MCAL_PORT_TARGET_STM32F103C8T6


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

void vPort_AFIO_mapSpi(uint8_t ucUnitNumber, uint8_t ucMapNumber);

void vPort_AFIO_mapI2C(uint8_t ucUnitNumber, uint8_t ucMapNumber);

void vPort_AFIO_mapUart(uint8_t ucUnitNumber, uint8_t ucMapNumber);

#endif /* HAL_OS_PORT_PORT_AFIO_H_ */


#endif /* Target checking */