/*
 * Port_EXTI.c
 *
 *  Created on: Jul 26, 2023
 *      Author: Ali Emad
 */

#include "stm32f103xb.h"
#include "stm32f1xx_hal.h"
#include "cmsis_gcc.h"

#include "stm32f1xx_ll_exti.h"
#include "stm32f1xx_ll_gpio.h"

const uint32_t puiExtiPinToAfioLine[5] = {
	LL_GPIO_AF_EXTI_LINE0 ,
	LL_GPIO_AF_EXTI_LINE1 ,
	LL_GPIO_AF_EXTI_LINE2 ,
	LL_GPIO_AF_EXTI_LINE3 ,
	LL_GPIO_AF_EXTI_LINE4
};

const uint32_t puiExtiPinToExtiLine[5] = {
		LL_EXTI_LINE_0 ,
		LL_EXTI_LINE_1 ,
		LL_EXTI_LINE_2 ,
		LL_EXTI_LINE_3 ,
		LL_EXTI_LINE_4
};
