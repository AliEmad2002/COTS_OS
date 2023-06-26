/*
 * Port_Timer.h
 *
 *  Created on: Jun 25, 2023
 *      Author: Ali Emad
 */

#ifndef HAL_OS_PORT_PORT_TIMER_H_
#define HAL_OS_PORT_PORT_TIMER_H_

#include "stm32f1xx.h"
#include "stm32f1xx_ll_tim.h"

/*******************************************************************************
 * Forced features for portability:
 * 		-	Timer clock source is forced to internal clock source.
 *
 * 		-	Timer counting direction is forced to up-counting.
 *
 * 		-	counter upper limit (the value that when reached by the counter, it
 * 			signals an OVF and reloads) is forced to the maximum counter value.
 *
 *		-	This driver assumes that all timers have the same internal clock source,
 *			as this is the case in most of our used MCUs. A major upper layer change
 *			may be needed when using a target that does not follow this rule.
 ******************************************************************************/

/*******************************************************************************
 * Target dependent configurations:
 ******************************************************************************/
/*
 * Prescaler value that when selected, and using the internal clock source (as
 * it is the forced source in this driver), the timer clocks at about 100kHz.
 *
 * It is a numeric value that actually expresses the prescaler, not an enumeration!
 */
#define uiPORT_TIM_PRESCALER_FOR_10_KHZ			7200
#define uiPORT_TIM_PRESCALER_FOR_100_KHZ		720
#define uiPORT_TIM_PRESCALER_FOR_500_KHZ		144

/*
 * In most cases, the selected prescaler among the previous definitions won't
 * result in exactly the said frequency, it would be some value near it.
 *
 * These near values must be user defined here (in Hz), as they are target dependent.
 */
#define uiPORT_TIM_FREQ_ACTUAL_FOR_10_KHZ			10000
#define uiPORT_TIM_FREQ_ACTUAL_FOR_100_KHZ			100000
#define uiPORT_TIM_FREQ_ACTUAL_FOR_500_KHZ			500000

/*
 * peripheral handles (i.e.: base pointers)
 * user must configure - if needed - in "Port_Timer.c", or user can create his own.
 */
extern TIM_TypeDef* const pxPortTimArr[];

/*
 * Timer counter register size (in bits).
 * User must configure in "Port_Timer.c"
 */
extern const uint32_t puiPortTimerCounterSizeInBits[];

/*******************************************************************************
 * Handlers:
 ******************************************************************************/
/*
 * Overflow handlers:
 * (If timer unit does not have a dedicated OVF_Handler, its main handler may be
 * used, as upper layers' codes will always use flag checking for portability.
 * The same applies if more than one timer share the same handler.
 */
#define port_TIM_OVF_HANDLER_0		TIM1_UP_IRQHandler
#define port_TIM_OVF_HANDLER_1		TIM2_IRQHandler
#define port_TIM_OVF_HANDLER_2		TIM3_IRQHandler
#define port_TIM_OVF_HANDLER_3		TIM4_IRQHandler

/*******************************************************************************
 * API functions:
 ******************************************************************************/
/*
 * Selects clock source to internal clock source.
 */
static inline void vPort_TIM_useInternalClockSource(uint8_t ucUnitNumber)
{
	 LL_TIM_SetClockSource(pxPortTimArr[ucUnitNumber], LL_TIM_CLOCKSOURCE_INTERNAL);
}

/*
 * Sets timer clock prescaler.
 *
 * argument "uiPrescaler" is one of "uiPORT_TIM_PRESCALER_FOR_xx_KHZ" defined values.
 */
static inline void vPort_TIM_setPrescaler(uint8_t ucUnitNumber, uint32_t uiPrescaler)
{
	LL_TIM_SetPrescaler(pxPortTimArr[ucUnitNumber], (uint16_t)uiPrescaler);
}

/*
 * makes timer run in normal mode, disabling any other previously selected mode.
 */
static inline void vPort_TIM_setModeNormal(uint8_t ucUnitNumber)
{

}

/*
 * Clears OVF flag
 */
static inline void vPort_TIM_clearOverflowFlag(uint8_t ucUnitNumber)
{
	LL_TIM_ClearFlag_UPDATE(pxPortTimArr[ucUnitNumber]);
}

/*
 * Checks OVF flag of a timer unit
 */
static inline uint8_t ucPort_TIM_getOverflowFlag(uint8_t ucUnitNumber)
{
	return LL_TIM_IsActiveFlag_UPDATE(pxPortTimArr[ucUnitNumber]);
}

/*
 * Enables OVF interrupt
 */
static inline void vPort_TIM_enableOverflowInterrupt(uint8_t ucUnitNumber)
{
	LL_TIM_EnableIT_UPDATE(pxPortTimArr[ucUnitNumber]);
}

/*
 * Disables OVF interrupt
 */
static inline void vPort_TIM_disableOverflowInterrupt(uint8_t ucUnitNumber)
{
	LL_TIM_DisableIT_UPDATE(pxPortTimArr[ucUnitNumber]);
}

/*
 * Sets counting direction up-counting.
 */
static inline void vPort_TIM_setCountingDirUp(uint8_t ucUnitNumber)
{
	LL_TIM_SetCounterMode(pxPortTimArr[ucUnitNumber], LL_TIM_COUNTERMODE_UP);
}

/*
 * Sets counter upper limit. (the value that when reached by the counter, it signals
 * an OVF and reloads.
 */
static inline void vPort_TIM_setCounterUpperLimit(uint8_t ucUnitNumber, uint32_t uiLim)
{
	LL_TIM_SetAutoReload(pxPortTimArr[ucUnitNumber], (uint16_t)uiLim);
}

/*
 * Writes (loads) counter register.
 * (if target forces using preload register instead of instant counter writing,
 * then write is made to the preload register)
 */
static inline void vPort_TIM_writeCounter(uint8_t ucUnitNumber, uint32_t uiCounter)
{
	LL_TIM_SetCounter(pxPortTimArr[ucUnitNumber], (uint16_t)uiCounter);
}

/*
 * reads counter register value.
 */
static inline uint32_t uiPort_TIM_readCounter(uint8_t ucUnitNumber)
{
	return LL_TIM_GetCounter(pxPortTimArr[ucUnitNumber]);
}

/*
 * Enables counter.
 */
static inline void vPort_TIM_enableCounter(uint8_t ucUnitNumber)
{
	 LL_TIM_EnableCounter(pxPortTimArr[ucUnitNumber]);
}

/*
 * Disables counter.
 */
static inline void vPort_TIM_disableCounter(uint8_t ucUnitNumber)
{
	LL_TIM_DisableCounter(pxPortTimArr[ucUnitNumber]);
}












#endif /* HAL_OS_PORT_PORT_TIMER_H_ */
