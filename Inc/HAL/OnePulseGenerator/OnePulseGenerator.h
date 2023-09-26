/*
 * OnePulseGenerator.h
 *
 *  Created on: Sep 26, 2023
 *      Author: Ali Emad
 */

#ifndef COTS_OS_INC_HAL_ONEPULSEGENERATOR_ONEPULSEGENERATOR_H_
#define COTS_OS_INC_HAL_ONEPULSEGENERATOR_ONEPULSEGENERATOR_H_

#include "MCAL_Port/Port_Timer.h"

#define uiHOS_ONE_PULSE_GENERATOR_MAX_FREQ_1_KHZ	uiPORT_TIM_PRESCALER_FOR_1_KHZ
#define uiHOS_ONE_PULSE_GENERATOR_MAX_FREQ_10_KHZ	uiPORT_TIM_PRESCALER_FOR_10_KHZ
#define uiHOS_ONE_PULSE_GENERATOR_MAX_FREQ_100_KHZ	uiPORT_TIM_PRESCALER_FOR_100_KHZ
#define uiHOS_ONE_PULSE_GENERATOR_MAX_FREQ_500_KHZ	uiPORT_TIM_PRESCALER_FOR_500_KHZ
#define uiHOS_ONE_PULSE_GENERATOR_MAX_FREQ_1_MHZ	uiPORT_TIM_PRESCALER_FOR_1_MHZ

/*
 * Each handle represents a unit which has multiple channels.
 */
typedef struct{
	/*		PUBLIC		*/
	uint8_t ucTimerUnitNumber;

	/*		PRIVATE		*/
	uint8_t ucLastEnabledChannel;
	SemaphoreHandle_t xSemaphore;
	StaticSemaphore_t xSemaphoreStatic;
}xHOS_OnePulseGenerator_t;

/*
 * Initializes handle.
 *
 * Notes:
 * 		-	Handle's "ucTimerUnitNumber" must be previously set to a valid value.
 *
 * 		-	"uiFreq" must be one of "uiHOS_ONE_PULSE_GENERATOR_MAX_FREQ_XX_HZ".
 */
void vHOS_OnePulseGenerator_init(	xHOS_OnePulseGenerator_t* pxHandle,
									uint32_t uiFreq	);

/*
 * Initializes channel's output pin.
 */
void vHOS_OnePulseGenerator_initChannel(	xHOS_OnePulseGenerator_t* pxHandle,
											uint8_t ucChannelNumber	);

/*
 * Generates pulse.
 */
void vHOS_OnePulseGenerator_generate(	xHOS_OnePulseGenerator_t* pxHandle,
										uint8_t ucChannelNumber,
										uint32_t uiHighTimeNanoSec	);


#endif /* COTS_OS_INC_HAL_ONEPULSEGENERATOR_ONEPULSEGENERATOR_H_ */
