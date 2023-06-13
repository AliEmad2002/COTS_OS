/*
 * Port_Interrupt.h
 *
 *  Created on: Jun 13, 2023
 *      Author: Ali Emad
 */

#ifndef HAL_OS_PORT_PORT_INTERRUPT_H_
#define HAL_OS_PORT_PORT_INTERRUPT_H_


/*
 * Interrupt driver (for example NVIC in case of using ARM-based target) is
 * included here, and its proper functions are called inside the following defined
 * wrapper functions.
 *
 * If such a function is not provided by the available driver, user may write it
 * inside the wrapper function (if too long to be inlined, remove the "inline"
 * identifier).
 *
 * If certain functionality is not even available in the target HW, its wrapper
 * is left empty. (for example: interrupt priority is not available in AVR MCUs)
 */
#include "stm32f103xb.h"
#include "stm32f1xx_hal.h"
#include "cmsis_gcc.h"

static inline void vHOS_Interrupt_enableGlobalInterrupt()
{
	__enable_irq();
}

static inline void vHOS_Interrupt_disableGlobalInterrupt()
{
	__disable_irq();
}

static inline void vHOS_Interrupt_enableIRQ(uint8_t ucIRQNumber)
{
	HAL_NVIC_EnableIRQ(ucIRQNumber);
}

static inline void vHOS_Interrupt_disableIRQ(uint8_t ucIRQNumber)
{
	HAL_NVIC_DisableIRQ(ucIRQNumber);
}


#endif /* HAL_OS_PORT_PORT_INTERRUPT_H_ */
