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

static inline void vPort_Interrupt_enableGlobalInterrupt()
{
	__enable_irq();
}

static inline void vPort_Interrupt_disableGlobalInterrupt()
{
	__disable_irq();
}

static inline void vPort_Interrupt_enableIRQ(uint8_t ucIRQNumber)
{
	HAL_NVIC_EnableIRQ(ucIRQNumber);
}

static inline void vPort_Interrupt_disableIRQ(uint8_t ucIRQNumber)
{
	HAL_NVIC_DisableIRQ(ucIRQNumber);
}

static inline void vPort_Interrupt_setPriority(uint8_t ucIRQNumber, uint8_t ucPri)
{
	NVIC_SetPriority(ucIRQNumber, ucPri);
}

/*******************************************************************************
 * IRQ numbers.
 * Notes:
 * 		-	Names must not be changed at all. Only value can be.
 * 		-	Some IRQs must be in arrays, such as these of repeated units, like
 * 			timers, SPI, UART, DMA, ... etc.
 * 		-	If the ported target has only one unit, an array of IRQs must be remained
 * 			as an array, it could be made of one element.
 ******************************************************************************/
extern const IRQn_Type pxPortInterruptSpiTxeIrqNumberArr[];

extern const IRQn_Type pxPortInterruptSpiRxneIrqNumberArr[];

extern const IRQn_Type pxPortInterruptTimerOvfIrqNumberArr[];

extern const IRQn_Type pxPortInterruptExtiIrqNumberArr[];

extern const IRQn_Type pxPortInterruptDmaIrqNumberArr[];

extern const IRQn_Type xPortInterruptUsbIrqNumber;

extern const IRQn_Type pxPortInterruptUartTxeIrqNumberArr[];

extern const IRQn_Type pxPortInterruptUartRxneIrqNumberArr[];

extern const IRQn_Type pxPortInterruptUartTcIrqNumberArr[];

extern const IRQn_Type xPortInterruptPwrPvdIrqNumber;

/*******************************************************************************
 * IRQ handler code enable.
 * 	As IRQ handlers are forced to be compiled (no optimization could be done to exclude
 * 	them from the compilation process whatsoever, even if the handler would never
 * 	be used in the compiled program) handlers are defined only when their corresponding
 * 	"ucPORT_INTERRUPT_IRQ_DEF_x" is defined first. Also, for any non used handlers,
 * 	application writer should comment their corresponding "ucPORT_INTERRUPT_IRQ_DEF_x"
 * 	to maintain reasonable flash usage.
 ******************************************************************************/
//#define ucPORT_INTERRUPT_IRQ_DEF_USB
//#define ucPORT_INTERRUPT_IRQ_DEF_TIM
//#define ucPORT_INTERRUPT_IRQ_DEF_EXTI
//#define ucPORT_INTERRUPT_IRQ_DEF_SPI
//#define ucPORT_INTERRUPT_IRQ_DEF_DMA
//#define ucPORT_INTERRUPT_IRQ_DEF_UART
#define ucPORT_INTERRUPT_IRQ_DEF_PWR_PVD












#endif /* HAL_OS_PORT_PORT_INTERRUPT_H_ */
