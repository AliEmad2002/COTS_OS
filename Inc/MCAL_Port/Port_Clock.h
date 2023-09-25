/*
 * Port_Clock.h
 *
 *  Created on: Jun 14, 2023
 *      Author: Ali Emad
 */

#ifndef HAL_OS_PORT_PORT_CLOCK_H_
#define HAL_OS_PORT_PORT_CLOCK_H_

#include "stm32f103xb.h"
#include "stm32f1xx_hal.h"
#include "LIB/Assert.h"

#define uiPORT_CLOCK_MAIN_HZ		72000000//(SystemCoreClock)

/*	Used with STM32 targets only	*/
#define uiPORT_CLOCK_AHB_DIV	RCC_SYSCLK_DIV1
#define uiPORT_CLOCK_APB1_DIV	RCC_HCLK_DIV2
#define uiPORT_CLOCK_APB2_DIV	RCC_SYSCLK_DIV1

/*
 * Initializes clock control peripheral, and enables the needed peripherals.
 * If the ported target does not have any, or has implementation of clock control
 * defined privately and called at startup, this wrapper is left empty.
 */
void vPort_Clock_init(void);



#endif /* HAL_OS_PORT_PORT_CLOCK_H_ */
