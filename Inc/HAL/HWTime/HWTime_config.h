/*
 * HWTime_config.h
 *
 *  Created on: Aug 28, 2023
 *      Author: Ali Emad
 */

#ifndef COTS_OS_INC_HAL_HWTime_HWTime_CONFIG_H_
#define COTS_OS_INC_HAL_HWTime_HWTime_CONFIG_H_

#include "MCAL_Port/Port_Timer.h"

#define ucHOS_HWTIME_TIMER_UNIT_NUMBER		1
#define uiHOS_HWTIME_TIMER_PRESCALER		(uiPORT_TIM_PRESCALER_FOR_500_KHZ)
#define uiHOS_HWTIME_TIMER_FREQ_ACTUAL		(uiPORT_TIM_FREQ_ACTUAL_FOR_500_KHZ)


#endif /* COTS_OS_INC_HAL_HWTime_HWTime_CONFIG_H_ */
