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

#define uiPORT_CLOCK_MAIN_HZ		(SystemCoreClock)

/*
 * Initializes clock control peripheral, and enables the needed peripherals.
 * If the ported target does not have any, or has implementation of clock control
 * defined privately and called at startup, this wrapper is left empty.
 */
static inline void vPort_Clock_init(void)
{
	SystemCoreClockUpdate();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_AFIO_CLK_ENABLE();
	__HAL_RCC_SPI1_CLK_ENABLE();
}



#endif /* HAL_OS_PORT_PORT_CLOCK_H_ */
