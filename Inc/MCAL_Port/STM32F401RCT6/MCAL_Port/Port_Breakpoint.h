/*
 * Port_Breakpoint.h
 *
 *  Created on: Jun 9, 2023
 *      Author: Ali Emad
 */

/*	Target checking	*/
#include "MCAL_Port/Port_Target.h"
#ifdef MCAL_PORT_TARGET_STM32F401RCT6


#ifndef HAL_OS_PORT_PORT_BREAKPOINT_H_
#define HAL_OS_PORT_PORT_BREAKPOINT_H_

/*
 * Breakpoint instruction (or its inline function/macro) is written inside the
 * following wrapper.
 *
 * If the target MCU does not support breakpoints, the wrapper is left empty.
 */


#define vPORT_BREAKPOINT()		asm volatile("BKPT #0")



#endif /* HAL_OS_PORT_PORT_BREAKPOINT_H_ */


#endif /* Target checking */