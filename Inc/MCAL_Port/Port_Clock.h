/*
 * Port_Clock.h
 *
 *  Created on: Jun 14, 2023
 *      Author: Ali Emad
 */

#ifndef HAL_OS_PORT_PORT_CLOCK_H_
#define HAL_OS_PORT_PORT_CLOCK_H_

#include "stm32f401xc.h"
#include "stm32f4xx_hal.h"
#include "LIB/Assert.h"

#define uiPORT_CLOCK_MAIN_HZ		84000000//(SystemCoreClock)

void vPort_Clock_initCpuClock(void);

void vPort_Clock_initPeriphClock(void);




#endif /* HAL_OS_PORT_PORT_CLOCK_H_ */
