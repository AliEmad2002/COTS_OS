/*
 * Port_Timer.c
 *
 *  Created on: Jun 26, 2023
 *      Author: Ali Emad
 */


#include "stm32f1xx.h"

TIM_TypeDef* const pxPortTimArr[] = {TIM1, TIM2, TIM3, TIM4};

const uint32_t puiPortTimerCounterSizeInBits[] = {16, 16, 16, 16};
