/*
 * Port_Breakpoint.h
 *
 *  Created on: Jun 9, 2023
 *      Author: Ali Emad
 */

#ifndef HAL_OS_PORT_PORT_BREAKPOINT_H_
#define HAL_OS_PORT_PORT_BREAKPOINT_H_

/*
 * Breakpoint instruction (or its inline function/macro) is written inside the
 * following wrapper.
 *
 * If the target MCU does not support breakpoints, the wrapper is left empty.
 */


#define vPort_Breakpoint()		\
{                               \
	asm volatile("BKPT #0");    \
}



#endif /* HAL_OS_PORT_PORT_BREAKPOINT_H_ */
