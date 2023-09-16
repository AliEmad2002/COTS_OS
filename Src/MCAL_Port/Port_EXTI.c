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

#include "MCAL_Port/Port_Interrupt.h"

const uint32_t puiPortExtiPinToAfioLineArr[16] = {
	LL_GPIO_AF_EXTI_LINE0 ,
	LL_GPIO_AF_EXTI_LINE1 ,
	LL_GPIO_AF_EXTI_LINE2 ,
	LL_GPIO_AF_EXTI_LINE3 ,
	LL_GPIO_AF_EXTI_LINE4 ,
	LL_GPIO_AF_EXTI_LINE5 ,
	LL_GPIO_AF_EXTI_LINE6 ,
	LL_GPIO_AF_EXTI_LINE7 ,
	LL_GPIO_AF_EXTI_LINE8 ,
	LL_GPIO_AF_EXTI_LINE9 ,
	LL_GPIO_AF_EXTI_LINE10 ,
	LL_GPIO_AF_EXTI_LINE11 ,
	LL_GPIO_AF_EXTI_LINE12 ,
	LL_GPIO_AF_EXTI_LINE13 ,
	LL_GPIO_AF_EXTI_LINE14 ,
	LL_GPIO_AF_EXTI_LINE15
};

const uint32_t puiPortExtiPinToExtiLineArr[16] = {
		LL_EXTI_LINE_0 ,
		LL_EXTI_LINE_1 ,
		LL_EXTI_LINE_2 ,
		LL_EXTI_LINE_3 ,
		LL_EXTI_LINE_4 ,
		LL_EXTI_LINE_5 ,
		LL_EXTI_LINE_6 ,
		LL_EXTI_LINE_7 ,
		LL_EXTI_LINE_8 ,
		LL_EXTI_LINE_9 ,
		LL_EXTI_LINE_10 ,
		LL_EXTI_LINE_11 ,
		LL_EXTI_LINE_12 ,
		LL_EXTI_LINE_13 ,
		LL_EXTI_LINE_14 ,
		LL_EXTI_LINE_15
};

void(*ppfPortExtiCallbackArr[16])(void*);
void* ppvPortExtiCallbackParamsArr[16];

#include "MCAL_Port/Port_EXTI.h"

/*******************************************************************************
 * ISRs:
 *
 * Notes
 * 		-	Define them as shown, target dependent.
 * 		-	Add clearing pending flag to the end of the ISR
 ******************************************************************************/
#ifdef ucPORT_INTERRUPT_IRQ_DEF_EXTI
void EXTI0_IRQHandler(void)
{
	ppfPortExtiCallbackArr[0](ppvPortExtiCallbackParamsArr[0]);
	LL_EXTI_ClearFlag_0_31(puiPortExtiPinToExtiLineArr[0]);
}

void EXTI1_IRQHandler(void)
{
	ppfPortExtiCallbackArr[1](ppvPortExtiCallbackParamsArr[1]);
	LL_EXTI_ClearFlag_0_31(puiPortExtiPinToExtiLineArr[1]);
}

void EXTI2_IRQHandler(void)
{
	ppfPortExtiCallbackArr[2](ppvPortExtiCallbackParamsArr[2]);
	LL_EXTI_ClearFlag_0_31(puiPortExtiPinToExtiLineArr[2]);
}

void EXTI3_IRQHandler(void)
{
	ppfPortExtiCallbackArr[3](ppvPortExtiCallbackParamsArr[3]);
	LL_EXTI_ClearFlag_0_31(puiPortExtiPinToExtiLineArr[3]);
}

void EXTI4_IRQHandler(void)
{
	ppfPortExtiCallbackArr[4](ppvPortExtiCallbackParamsArr[4]);
	LL_EXTI_ClearFlag_0_31(puiPortExtiPinToExtiLineArr[4]);
}

void EXTI9_5_IRQHandler(void)
{
	for (uint8_t i = 5; i <= 9; i++)
	{
		if (LL_EXTI_ReadFlag_0_31(puiPortExtiPinToExtiLineArr[i]))
		{
			ppfPortExtiCallbackArr[i](ppvPortExtiCallbackParamsArr[i]);
			LL_EXTI_ClearFlag_0_31(puiPortExtiPinToExtiLineArr[i]);
		}
	}
}

void EXTI15_10_IRQHandler(void)
{
	for (uint8_t i = 10; i <= 15; i++)
	{
		if (LL_EXTI_ReadFlag_0_31(puiPortExtiPinToExtiLineArr[i]))
		{
			ppfPortExtiCallbackArr[i](ppvPortExtiCallbackParamsArr[i]);
			LL_EXTI_ClearFlag_0_31(puiPortExtiPinToExtiLineArr[i]);
		}
	}
}
#endif






