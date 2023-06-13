/*
 * Port_Interrupt.c
 *
 *  Created on: Jun 13, 2023
 *      Author: Ali Emad
 */

#include "stm32f103xb.h"
#include "stm32f1xx_hal.h"
#include "cmsis_gcc.h"

const IRQn_Type pxPortInterruptSpiIrqNumberArr[] = {SPI1_IRQn, SPI2_IRQn};
