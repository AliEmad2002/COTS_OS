/*
 * Port_SPI.c
 *
 *  Created on: Jun 13, 2023
 *      Author: Ali Emad
 */

#include "stm32f103xb.h"
#include "stm32f1xx_hal.h"

SPI_TypeDef* const xPortSpiArr[] = {SPI1, SPI2};
