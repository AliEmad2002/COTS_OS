/*
 * Port_I2C.c
 *
 *  Created on: Jul 9, 2023
 *      Author: Ali Emad
 */

#include "stm32f103xb.h"
#include "stm32f1xx_hal.h"

I2C_TypeDef* const pxPortI2cArr[] = {I2C1, I2C2};
