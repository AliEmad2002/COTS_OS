/*
 * Port_Print.h
 *
 *  Created on: Jun 9, 2023
 *      Author: Ali Emad
 */

/*	Target checking	*/
#include "MCAL_Port/Port_Target.h"
#ifdef MCAL_PORT_TARGET_STM32F401RCT6


#ifndef HAL_OS_PORT_PORT_PRINT_H_
#define HAL_OS_PORT_PORT_PRINT_H_


/*
 * (Print, Trace, .. etc) driver of target MCU is included here, and its proper
 * functions are called inside the following defined wrapper macro.
 *
 * If such a function is not provided by the available driver, user may write it
 * inside the wrapper macro (if too long to be inlined as a macro, define as a function).
 */
#include "stm32f401xc.h"
#include "stm32f4xx_hal.h"
#include <stdio.h>


#define vPort_Print(...)	printf(__VA_ARGS__)

#endif /* HAL_OS_PORT_PORT_PRINT_H_ */


#endif /* Target checking */
