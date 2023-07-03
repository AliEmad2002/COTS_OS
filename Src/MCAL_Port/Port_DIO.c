/*
 * Port_DIO.c
 *
 *  Created on: Jun 9, 2023
 *      Author: Ali Emad
 */

#include "stm32f103xb.h"
#include "stm32f1xx_hal.h"

GPIO_TypeDef* const pxPortDioPortArr[] = {GPIOA, GPIOB, GPIOC};
