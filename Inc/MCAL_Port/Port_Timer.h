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

extern const uint32_t puiChannels[];

/*
 * Timer counter register size (in bits).
 * User must configure in "Port_Timer.c"
 */
extern const uint8_t pucPortTimerCounterSizeInBits[];


/*******************************************************************************
 * API functions:
 ******************************************************************************/
/*
 * Selects clock source to internal clock source.
 */
#define vPORT_TIM_USE_INTERNAL_CLOCK_SOURCE(ucUnitNumber)	\
	 (	LL_TIM_SetClockSource(pxPortTimArr[(ucUnitNumber)], LL_TIM_CLOCKSOURCE_INTERNAL)	)

/*
 * Sets timer clock prescaler.
 *
 * argument "uiPrescaler" is one of "uiPORT_TIM_PRESCALER_FOR_xx_KHZ" defined values.
 */
#define VPORT_TIM_SET_PRESCALER(ucUnitNumber, uiPrescaler)	\
	(	LL_TIM_SetPrescaler(pxPortTimArr[(ucUnitNumber)], (uint16_t)(uiPrescaler))	)

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
#define vPORT_TIM_ENABLE_OVF_INTERRUPT(ucUnitNumber)	\
	(	LL_TIM_EnableIT_UPDATE(pxPortTimArr[(ucUnitNumber)])	)

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
#define vPORT_TIM_ENABLE_CC_INTERRUPT(ucUnitNumber)	\
	(	LL_TIM_EnableIT_CC1(pxPortTimArr[(ucUnitNumber)])	)

/*
 * Disables Capture / Compare interrupt
 */
#define vPORT_TIM_DISABLE_CC_INTERRUPT(ucUnitNumber)	\
	(	LL_TIM_DisableIT_CC1(pxPortTimArr[(ucUnitNumber)])	)

/*
 * Checks if Capture / Compare interrupt is enabled
 */
#define ucPORT_TIM_IS_CC_INTERRUPT_ENABLED(ucUnitNumber)	\
	(	LL_TIM_IsEnabledIT_CC1(pxPortTimArr[(ucUnitNumber)])	)

/*
 * Sets OVF interrupt callback
 */
void vPort_TIM_setOvfCallback(	uint8_t ucUnitNumber,
								void (*pfCallback)(void*),
								void* pvParams	);

/*
 * Sets Capture/Compare interrupt callback
 */
void vPort_TIM_setCcCallback(	uint8_t ucUnitNumber,
								void (*pfCallback)(void*),
								void* pvParams	);

/*
 * Sets counting direction up-counting.
 */
#define vPORT_TIM_SET_COUNTING_DIR_UP(ucUnitNumber)	\
	(	LL_TIM_SetCounterMode(pxPortTimArr[(ucUnitNumber)], LL_TIM_COUNTERMODE_UP)	)

/*
 * Sets counter upper limit. (the value that when reached by the counter, it signals
 * an OVF and reloads.
 */
#define vPORT_TIM_SET_COUNTER_UPPER_LIMIT(ucUnitNumber, uiLim)	\
	(	LL_TIM_SetAutoReload(pxPortTimArr[(ucUnitNumber)], (uint16_t)(uiLim))	)

/*
 * Writes (loads) counter register.
 * (if target forces using preload register instead of instant counter writing,
 * then write is made to the preload register)
 */
#define vPORT_TIM_WRITE_COUNTER(ucUnitNumber, uiCounter)	\
	(	LL_TIM_SetCounter(pxPortTimArr[(ucUnitNumber)], (uint16_t)(uiCounter))	)

/*
 * reads counter register value.
 */
#define uiPORT_TIM_READ_COUNTER(ucUnitNumber)	\
	(LL_TIM_GetCounter(pxPortTimArr[(ucUnitNumber)]))

/*
 * Enables counter.
 */
#define vPORT_TIM_ENABLE_COUNTER(ucUnitNumber)	\
	 (	LL_TIM_EnableCounter(pxPortTimArr[(ucUnitNumber)])	)

/*
 * Disables counter.
 */
#define vPORT_TIM_DISABLE_COUNTER(ucUnitNumber)	\
	 (	LL_TIM_EnableCounter(pxPortTimArr[(ucUnitNumber)])	)

/*
 * Sets output compare register.
 */
#define vPORT_TIM_WRITE_OC_REGISTER(ucUnitNumber, uiVal)	\
	(	LL_TIM_OC_SetCompareCH1(pxPortTimArr[(ucUnitNumber)], (uint16_t)(uiVal))	)

/*
 * Enables Capture / Compare channel.
 */
#define vPORT_TIM_ENABLE_CC_CHANNEL(ucUnitNumber, ucChannelNumber)	\
	(	LL_TIM_CC_EnableChannel(pxPortTimArr[(ucUnitNumber)], puiChannels[(ucChannelNumber)])	)

/*
 * Disables Capture / Compare channel.
 */
#define vPORT_TIM_DISABLE_CC_CHANNEL(ucUnitNumber, ucChannelNumber)	\
	(	LL_TIM_CC_DisableChannel(pxPortTimArr[(ucUnitNumber)], puiChannels[(ucChannelNumber)])	)

/*
 * Sets overflow frequency as near as possible to the requested value by changing
 * prescaler and maximum value.
 *
 * If succeeded, the function returns actual configured frequency in Hz. Otherwise,
 * if failed, the function returns 0.
 *
 *
 */
uint32_t uiPort_TIM_setOvfFreq(uint8_t ucUnitNumber, uint32_t uiFreq);

/*
 * Initializes timer channel for PWM output.
 */
void vPort_TIM_initChannelPwmOutput(uint8_t ucUnitNumber, uint8_t ucChannelNumber);

/*
 * Sets PWM duty of a timer channel.
 *
 * Notes:
 * 		-	Channels' GPIO configuration must be done separately (if needed).
 */
void vPort_TIM_setPwmDuty(	uint8_t  ucUnitNumber,
							uint8_t ucChannelNumber,
							uint16_t usDuty	);

/*
 * Initializes a timer unit for OPM (One-Pulse Mode).
 *
 * Notes:
 * 		-	"uiPrescaler" must be of of the "uiPORT_TIM_PRESCALER_FOR_xx_KHZ" macros.
 *
 * 		-	Once a timer unit is initialized for OPM, the unit and its channels
 * 			can only be used for generating OPM signals.
 *
 * 		-	Channels' GPIO configuration must be done separately (if needed).
 *
 * 		-	In order for OPM to work properly, no timer functions AT ALL should
 * 			be used after initializing, except for the "vPort_TIM_generateOnePulse()"
 * 			function.
 */
void vPort_TIM_initOPM(uint8_t ucUnitNumber, uint32_t uiPrescaler);

/*
 * Generates OPM (One-Pulse Mode) positive pulse on a timer channel.
 *
 * Notes:
 * 		-	"1 / uiTimeNanoSeconds" must be of a frequency less than timer's frequency
 * 			(which is configured in "vPort_TIM_initOPM()").
 *
 *		-	Timer resolution may not be enough to achieve the requested
 *			"uiTimeNanoSeconds" precisely. If so, use a higher timer frequency
 *			when initializing (using "vPort_TIM_initOPM()").
 *
 *		-	Channel's capture / compare must be enabled first. (Notice that enabling
 *			capture / compare of multiple of the same timer unit, results in generating
 *			the pulse on all of them. Therefore, only one channel may be enabled at
 *			a time. A higher level SW could handle this.
 *
 *		-	So far, this driver can only generate OPM pulse on a single channel
 *			at a time. A mutex  may be used on the timer unit to assure it is used
 *			by one channel at a time.
 */
void vPort_TIM_generateOnePulse(	uint8_t ucUnitNumber,
									uint8_t ucChannelNumber,
									uint32_t uiTimeNanoSeconds);






#endif /* HAL_OS_PORT_PORT_TIMER_H_ */
