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


const uint32_t puiPortExtiPinToAfioLineArr[5] = {
	LL_GPIO_AF_EXTI_LINE0 ,
	LL_GPIO_AF_EXTI_LINE1 ,
	LL_GPIO_AF_EXTI_LINE2 ,
	LL_GPIO_AF_EXTI_LINE3 ,
	LL_GPIO_AF_EXTI_LINE4
};

const uint32_t puiPortExtiPinToExtiLineArr[5] = {
		LL_EXTI_LINE_0 ,
		LL_EXTI_LINE_1 ,
		LL_EXTI_LINE_2 ,
		LL_EXTI_LINE_3 ,
		LL_EXTI_LINE_4
};

void(*ppfPortExtiCallbackArr[5])(void*);
void* ppvPortExtiCallbackParamsArr[5];

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
#endif
