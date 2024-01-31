/*
 * Port_Clock.h
 *
 *  Created on: Jun 14, 2023
 *      Author: Ali Emad
 */

/*	Target checking	*/
#include "MCAL_Port/Port_Target.h"
#ifdef MCAL_PORT_TARGET_STM32F103C8T6


#ifndef HAL_OS_PORT_PORT_CLOCK_H_
#define HAL_OS_PORT_PORT_CLOCK_H_

#include "stm32f103xb.h"
#include "stm32f1xx_hal.h"
#include "LIB/Assert.h"

#define uiPORT_CLOCK_MAIN_HZ		72000000//(SystemCoreClock)

/*	Used with STM32 targets only	*/
#define uiPORT_CLOCK_AHB_DIV	1
#define uiPORT_CLOCK_APB1_DIV	2
#define uiPORT_CLOCK_APB2_DIV	1
#define uiPORT_CLOCK_ADC_DIV	6

void vPort_Clock_initCpuClock(void);

void vPort_Clock_initPeriphClock(void);




#endif /* HAL_OS_PORT_PORT_CLOCK_H_ */


#endif /* Target checking */