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
 * it is the forced source in this driver), the timer clocks at about the frequency
 * defined in macro's name.
 *
 * It is a numeric value that actually expresses the prescaler, not an enumeration!
 */
#define uiPORT_TIM_PRESCALER_FOR_1_KHZ			72000
#define uiPORT_TIM_PRESCALER_FOR_10_KHZ			7200
#define uiPORT_TIM_PRESCALER_FOR_100_KHZ		720
#define uiPORT_TIM_PRESCALER_FOR_500_KHZ		144
#define uiPORT_TIM_PRESCALER_FOR_1_MHZ			72

/*
 * In most cases, the selected prescaler among the previous definitions won't
 * result in exactly the said frequency, it would be some value near it.
 *
 * These near values must be user defined here (in Hz), as they are target dependent.
 */
#define uiPORT_TIM_FREQ_ACTUAL_FOR_1_KHZ			1000
#define uiPORT_TIM_FREQ_ACTUAL_FOR_10_KHZ			10000
#define uiPORT_TIM_FREQ_ACTUAL_FOR_100_KHZ			100000
#define uiPORT_TIM_FREQ_ACTUAL_FOR_500_KHZ			500000
#define uiPORT_TIM_FREQ_ACTUAL_FOR_1_MHZ			1000000

/*
 * peripheral handles (i.e.: base pointers)
 * user must configure - if needed - in "Port_Timer.c", or user can create his own.
 */
extern TIM_TypeDef* const pxPortTimArr[];

/*
 * Timer counter register size (in bits).
 * User must configure in "Port_Timer.c"
 */
extern const uint8_t pucPortTimerCounterSizeInBits[];

/*
 * ISR callbacks and their params.
 */
extern void (*ppfPortTimerOvfCallbackArr[4])(void*);
extern void* ppvPortTimerOvfCallbackParamsArr[4];

extern void (*ppfPortTimerCompareCallbackArr[4])(void*);
extern void* ppvPortTimerCompareCallbackParamsArr[4];


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
#define vPORT_TIM_CLEAR_OVF_FLAG(ucUnitNumber)	\
	(LL_TIM_ClearFlag_UPDATE(pxPortTimArr[(ucUnitNumber)]))

/*
 * Clears Capture/Compare flag
 */
#define vPORT_TIM_CLEAR_CC_FLAG(ucUnitNumber)	\
	(LL_TIM_ClearFlag_CC1(pxPortTimArr[(ucUnitNumber)]))

/*
 * Reads OVF flag of a timer unit
 */
#define ucPORT_TIM_GET_OVF_FLAG(ucUnitNumber)	\
	(LL_TIM_IsActiveFlag_UPDATE(pxPortTimArr[(ucUnitNumber)]))

/*
 * Reads Capture/Compare flag of a timer unit
 */
#define ucPORT_TIM_GET_CC_FLAG(ucUnitNumber)	\
	(LL_TIM_IsActiveFlag_CC1(pxPortTimArr[(ucUnitNumber)]))

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
#define vPORT_TIM_DISABLE_OVF_INTERRUPT(ucUnitNumber)	\
	(	LL_TIM_DisableIT_UPDATE(pxPortTimArr[(ucUnitNumber)])	)

/*
 * Checks if OVF interrupt is enabled
 */
#define ucPORT_TIM_IS_OVF_INTERRUPT_ENABLED(ucUnitNumber)	\
	(	LL_TIM_IsEnabledIT_UPDATE(pxPortTimArr[(ucUnitNumber)])	)

/*
 * Enables Capture / Compare interrupt
 */
static inline void vPort_TIM_enableCcInterrupt(uint8_t ucUnitNumber)
{
	LL_TIM_EnableIT_CC1(pxPortTimArr[ucUnitNumber]);
}

/*
 * Disables Capture / Compare interrupt
 */
static inline void vPort_TIM_disableCcInterrupt(uint8_t ucUnitNumber)
{
	LL_TIM_DisableIT_CC1(pxPortTimArr[ucUnitNumber]);
}

/*
 * Checks if Capture / Compare interrupt is enabled
 */
#define ucPORT_TIM_IS_CC_INTERRUPT_ENABLED(ucUnitNumber)	\
	(	LL_TIM_IsEnabledIT_CC1(pxPortTimArr[(ucUnitNumber)])	)

/*
 * Sets OVF interrupt callback
 */
static inline void vPort_TIM_setOvfCallback(	uint8_t ucUnitNumber,
												void (*pfCallback)(void*),
												void* pvParams	)
{
	ppfPortTimerOvfCallbackArr[ucUnitNumber] = pfCallback;
	ppvPortTimerOvfCallbackParamsArr[ucUnitNumber] = pvParams;
}

/*
 * Sets Capture/Compare interrupt callback
 */
static inline void vPort_TIM_setCcCallback(	uint8_t ucUnitNumber,
											void (*pfCallback)(void*),
											void* pvParams	)
{
	ppfPortTimerCompareCallbackArr[ucUnitNumber] = pfCallback;
	ppvPortTimerCompareCallbackParamsArr[ucUnitNumber] = pvParams;
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
#define uiPORT_TIM_READ_COUNTER(ucUnitNumber)	\
	(LL_TIM_GetCounter(pxPortTimArr[(ucUnitNumber)]))

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

/*
 * Sets output compare register.
 */
static inline void vPort_TIM_writeOcRegister(uint8_t ucUnitNumber, uint32_t uiVal)
{
	LL_TIM_OC_SetCompareCH1(pxPortTimArr[ucUnitNumber], (uint16_t)uiVal);
}











#endif /* HAL_OS_PORT_PORT_TIMER_H_ */
