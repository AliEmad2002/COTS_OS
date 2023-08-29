/*
 * Port_Timer.c
 *
 *  Created on: Jun 26, 2023
 *      Author: Ali Emad
 */


#include "stm32f1xx.h"

TIM_TypeDef* const pxPortTimArr[] = {TIM1, TIM2, TIM3, TIM4};

const uint32_t puiPortTimerCounterSizeInBits[] = {16, 16, 16, 16};

void (*ppfPortTimerOvfCallbackArr[4])(void*);
void* ppvPortTimerOvfCallbackParamsArr[4];

#include "MCAL_Port/Port_Timer.h"

/*******************************************************************************
 * ISRs:
 *
 * Notes
 * 		-	Define them as shown, target dependent.
 * 		-	Add clearing pending flag to the end of the ISR
 ******************************************************************************/
void TIM1_UP_IRQHandler(void)
{
	ppfPortTimerOvfCallbackArr[0](ppvPortTimerOvfCallbackParamsArr[0]);
	vPort_TIM_clearOverflowFlag(0);
}

void TIM2_IRQHandler(void)
{
	if (!ucPORT_TIM_GET_OVF_FLAG(1))
		return;

	ppfPortTimerOvfCallbackArr[1](ppvPortTimerOvfCallbackParamsArr[1]);
	vPort_TIM_clearOverflowFlag(1);
}

//void TIM3_IRQHandler(void)
//{
//	if (!ucPort_TIM_getOverflowFlag(2))
//		return;
//
//	ppfPortTimerOvfCallbackArr[2](ppvPortTimerOvfCallbackParamsArr[2]);
//	vPort_TIM_clearOverflowFlag(2);
//}

void TIM4_IRQHandler(void)
{
	if (!ucPORT_TIM_GET_OVF_FLAG(3))
		return;

	ppfPortTimerOvfCallbackArr[3](ppvPortTimerOvfCallbackParamsArr[3]);
	vPort_TIM_clearOverflowFlag(3);
}





