/*
 * Port_Timer.c
 *
 *  Created on: Jun 26, 2023
 *      Author: Ali Emad
 */


#include "stm32f1xx.h"

TIM_TypeDef* const pxPortTimArr[] = {TIM1, TIM2, TIM3, TIM4};

const uint8_t pucPortTimerCounterSizeInBits[] = {16, 16, 16, 16};

#include "MCAL_Port/Port_Timer.h"
#include "MCAL_Port/Port_Interrupt.h"

/*******************************************************************************
 * ISRs:
 *
 * Notes
 * 		-	Define them as shown, target dependent.
 * 		-	Add clearing pending flag to the end of the ISR
 ******************************************************************************/
#ifdef ucPORT_INTERRUPT_IRQ_DEF_TIM

void (*ppfPortTimerOvfCallbackArr[4])(void*);
void* ppvPortTimerOvfCallbackParamsArr[4];

void (*ppfPortTimerCompareCallbackArr[4])(void*);
void* ppvPortTimerCompareCallbackParamsArr[4];


void vPort_TIM_setOvfCallback(	uint8_t ucUnitNumber,
												void (*pfCallback)(void*),
												void* pvParams	)
{
	ppfPortTimerOvfCallbackArr[ucUnitNumber] = pfCallback;
	ppvPortTimerOvfCallbackParamsArr[ucUnitNumber] = pvParams;
}

void vPort_TIM_setCcCallback(	uint8_t ucUnitNumber,
								void (*pfCallback)(void*),
								void* pvParams	)
{
	ppfPortTimerCompareCallbackArr[ucUnitNumber] = pfCallback;
	ppvPortTimerCompareCallbackParamsArr[ucUnitNumber] = pvParams;
}

void TIM1_UP_IRQHandler(void)
{
	ppfPortTimerOvfCallbackArr[0](ppvPortTimerOvfCallbackParamsArr[0]);
	vPORT_TIM_CLEAR_OVF_FLAG(0);
}

void TIM1_CC_IRQHandler(void)
{
	ppfPortTimerCompareCallbackArr[0](ppvPortTimerCompareCallbackParamsArr[0]);
	vPORT_TIM_CLEAR_CC_FLAG(0);
}

void TIM2_IRQHandler(void)
{
#define UNIT_NUM	1

	if (ucPORT_TIM_GET_OVF_FLAG(UNIT_NUM) && ucPORT_TIM_IS_OVF_INTERRUPT_ENABLED(UNIT_NUM))
	{
		ppfPortTimerOvfCallbackArr[UNIT_NUM](ppvPortTimerOvfCallbackParamsArr[UNIT_NUM]);
		vPORT_TIM_CLEAR_OVF_FLAG(UNIT_NUM);
	}

	if (ucPORT_TIM_GET_CC_FLAG(UNIT_NUM) && ucPORT_TIM_IS_CC_INTERRUPT_ENABLED(UNIT_NUM))
	{
		ppfPortTimerCompareCallbackArr[UNIT_NUM](ppvPortTimerCompareCallbackParamsArr[UNIT_NUM]);
		vPORT_TIM_CLEAR_CC_FLAG(UNIT_NUM);
	}

#undef UNIT_NUM
}

void TIM3_IRQHandler(void)
{
#define UNIT_NUM	2

	if (ucPORT_TIM_GET_OVF_FLAG(UNIT_NUM))
	{
		ppfPortTimerOvfCallbackArr[UNIT_NUM](ppvPortTimerOvfCallbackParamsArr[UNIT_NUM]);
		vPORT_TIM_CLEAR_OVF_FLAG(UNIT_NUM);
	}

	if (ucPORT_TIM_GET_CC_FLAG(UNIT_NUM))
	{
		ppfPortTimerCompareCallbackArr[UNIT_NUM](ppvPortTimerCompareCallbackParamsArr[UNIT_NUM]);
		vPORT_TIM_CLEAR_CC_FLAG(UNIT_NUM);
	}

#undef UNIT_NUM
}

void TIM4_IRQHandler(void)
{
#define UNIT_NUM	3

	if (ucPORT_TIM_GET_OVF_FLAG(UNIT_NUM))
	{
		ppfPortTimerOvfCallbackArr[UNIT_NUM](ppvPortTimerOvfCallbackParamsArr[UNIT_NUM]);
		vPORT_TIM_CLEAR_OVF_FLAG(UNIT_NUM);
	}

	if (ucPORT_TIM_GET_CC_FLAG(UNIT_NUM))
	{
		ppfPortTimerCompareCallbackArr[UNIT_NUM](ppvPortTimerCompareCallbackParamsArr[UNIT_NUM]);
		vPORT_TIM_CLEAR_CC_FLAG(UNIT_NUM);
	}

#undef UNIT_NUM
}

#endif




