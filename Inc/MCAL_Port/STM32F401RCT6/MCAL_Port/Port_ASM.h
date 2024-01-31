/*
 * Port_ASM.h
 *
 *  Created on: Aug 27, 2023
 *      Author: Ali Emad
 *
 * This driver implements special function instructions to accelerate program
 * execution.
 *
 * For any function that the target CPU does not support by a direct instruction,
 * multiple instructions / lines of C are used instead.
 */

/*	Target checking	*/
#include "MCAL_Port/Port_Target.h"
#ifdef MCAL_PORT_TARGET_STM32F401RCT6


#ifndef COTS_OS_INC_MCAL_PORT_PORT_ROTATE_H_
#define COTS_OS_INC_MCAL_PORT_PORT_ROTATE_H_


///*
// * Reverses byte order in each 16-bit, of a given 32-bit value.
// */
//__attribute__((always_inline)) inline
//static inline uint32_t uiPort_ASM_reverseEach16(uint32_t uiX)
//{
//  uint32_t result;
//
//  asm volatile(
//		  	 	 "REV16 %0, %1"		:
//		  	  	  "=r"	(result)	:
//				  "r"	(uiX)		);
//
//  return result;
//}
//
///*
// * Reverses byte order of a given 16-bit value.
// */
//__attribute__((always_inline)) inline
//static inline uint16_t uiPort_ASM_reverse16(uint16_t uiX)
//{
//  uint32_t result = uiX;
//
//  asm volatile(
//		  	 	 "REV16 %0, %1"		:
//		  	  	  "=r"	(result)	:
//				  "r"	(result)	);
//
//  return (uint16_t)result;
//}

/*
 * Executes an "NOP" instruction.
 */
#define vPORT_ASM_NOP()		asm volatile("NOP")











#endif /* COTS_OS_INC_MCAL_PORT_PORT_ROTATE_H_ */


#endif /* Target checking */