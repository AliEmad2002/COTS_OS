/*
 * Port_Timer.c
 *
 *  Created on: Jun 26, 2023
 *      Author: Ali Emad
 */

/*	Target checking	*/
#include "MCAL_Port/Port_Target.h"
#ifdef MCAL_PORT_TARGET_STM32F103C8T6



#include "stm32f1xx.h"
#include "stm32f1xx_ll_tim.h"

TIM_TypeDef* const pxPortTimArr[] = {TIM1, TIM2, TIM3, TIM4};

const uint32_t puiChannels[] = {
	LL_TIM_CHANNEL_CH1, LL_TIM_CHANNEL_CH2, LL_TIM_CHANNEL_CH3, LL_TIM_CHANNEL_CH4};

const uint8_t pucPortTimerCounterSizeInBits[] = {16, 16, 16, 16};

#include "MCAL_Port/Port_Timer.h"
#include "MCAL_Port/Port_Clock.h"
#include "MCAL_Port/Port_Interrupt.h"
#include "MCAL_Port/Port_Timer.h"

/*******************************************************************************
 * API functions:
 ******************************************************************************/
/*
 * See header for info.
 */
 uint32_t uiPort_TIM_setOvfFreq(uint8_t ucUnitNumber, uint32_t uiFreq)
 {
	/*	Disable counter	*/
	vPORT_TIM_DISABLE_COUNTER(ucUnitNumber);

	/*	get internal clock source frequency	*/
	uint32_t uiClkInt = uiPORT_CLOCK_MAIN_HZ;

	/*
	 * since: Frequency = clk_int / (ARR * prescaler),
	 * then: product of ARR * Prescaler = clk_int / Frequency
	 */
	uint32_t uiPrescalerArrProduct = uiClkInt / uiFreq;

	/*
	 * Now we want the combination of ARR and Prescaler that when multiplied they
	 * give the requested overflow frequency with minimum error.
	 */
	uint16_t usArrAtMinErr = 0;
	uint32_t uiMinErr = 65535;

	for (uint32_t uiArr = 65535; uiArr > 1; uiArr--)
	{
		uint32_t uiErr = uiPrescalerArrProduct % uiArr;

		if (uiErr < uiMinErr)
		{
			uiMinErr = uiErr;
			usArrAtMinErr = uiArr;
		}
	}

	uint32_t uiPrescalerAtMinErr = uiPrescalerArrProduct / usArrAtMinErr;
	uint32_t uiFreqActual;

	/*	If the prescaler value is valid	*/
	if (uiPrescalerAtMinErr > 0 && uiPrescalerAtMinErr <= 65535)
	{
		/*	Set ARR and prescaler values in timer unit	*/
		LL_TIM_SetAutoReload(pxPortTimArr[ucUnitNumber], usArrAtMinErr);
		LL_TIM_SetPrescaler(pxPortTimArr[ucUnitNumber], uiPrescalerAtMinErr - 1);

		/*	calculate actual frequency	*/
		uiFreqActual = uiClkInt / (usArrAtMinErr * uiPrescalerAtMinErr);
	}

	/*	otherwise, return failure	*/
	else
		return 0;

	/*	select up-counting mode	*/
	vPORT_TIM_SET_COUNTING_DIR_UP(ucUnitNumber);

	/*	load TCNT with zero	*/
	LL_TIM_SetCounter(pxPortTimArr[ucUnitNumber], 0);

	/*	load repetition counter with zero	*/
	LL_TIM_SetRepetitionCounter(pxPortTimArr[ucUnitNumber], 0);

	/*	Enable counter	*/
	vPORT_TIM_ENABLE_COUNTER(ucUnitNumber);

	return uiFreqActual;
}

/*
 * See header for info.
 */
void vPort_TIM_initChannelPwmOutput(uint8_t ucUnitNumber, uint8_t ucChannelNumber)
{
	/*	Disable counter	*/
	vPORT_TIM_DISABLE_COUNTER(ucUnitNumber);

	/*	enable output compare fast	*/
	LL_TIM_OC_EnableFast(pxPortTimArr[ucUnitNumber], puiChannels[ucChannelNumber]);

	/*	enable output compare preload	*/
	LL_TIM_OC_EnablePreload(pxPortTimArr[ucUnitNumber], puiChannels[ucChannelNumber]);

	/*	select output compare mode to PWM	*/
	LL_TIM_OC_SetMode(	pxPortTimArr[ucUnitNumber],
						puiChannels[ucChannelNumber],
						LL_TIM_OCMODE_PWM1);

	/*	enable capture/compare channel	*/
	LL_TIM_CC_EnableChannel(pxPortTimArr[ucUnitNumber], puiChannels[ucChannelNumber]);

	/*	if the selected timer unit is an advanced timer (not GP timer), enable outputs	*/
	if (ucUnitNumber == 0)
	{
		LL_TIM_EnableAllOutputs(pxPortTimArr[ucUnitNumber]);
	}

	/*	Enable counter	*/
	vPORT_TIM_ENABLE_COUNTER(ucUnitNumber);
}

/*
 * See header for info.
 */
void vPort_TIM_setPwmDuty(	uint8_t  ucTimerNumber,
							uint8_t ucChannelNumber,
							uint16_t usDuty	)
{
	/*	calculate CCR value	*/
	uint16_t usCCR = ( (uint32_t)usDuty * LL_TIM_GetAutoReload(pxPortTimArr[ucTimerNumber]) ) / 65535;

	/*	write it to "ucChannelNumber"	*/
	switch(ucChannelNumber)
	{
	case 0:
		LL_TIM_OC_SetCompareCH1(pxPortTimArr[ucTimerNumber], usCCR);
		break;

	case 1:
		LL_TIM_OC_SetCompareCH2(pxPortTimArr[ucTimerNumber], usCCR);
		break;

	case 2:
		LL_TIM_OC_SetCompareCH3(pxPortTimArr[ucTimerNumber], usCCR);
		break;

	case 3:
		LL_TIM_OC_SetCompareCH4(pxPortTimArr[ucTimerNumber], usCCR);
		break;
	}
}

/*
 * See header for info.
 */
void vPort_TIM_initOPM(uint8_t ucUnitNumber, uint32_t uiPrescaler)
{
	/*	Disable counter	*/
	vPORT_TIM_DISABLE_COUNTER(ucUnitNumber);

	/*	Select OPM	*/
	LL_TIM_SetOnePulseMode(pxPortTimArr[ucUnitNumber], LL_TIM_ONEPULSEMODE_SINGLE);

	/*	Set prescaler to the passed value	*/
	LL_TIM_SetPrescaler(pxPortTimArr[ucUnitNumber], uiPrescaler - 1);

	/*	select up-counting mode	*/
	vPORT_TIM_SET_COUNTING_DIR_UP(ucUnitNumber);

	/*	For every channel of the four	*/
	for (uint32_t i = 0; i < 4; i++)
	{
		/*	enable output compare fast	*/
		LL_TIM_OC_EnableFast(pxPortTimArr[ucUnitNumber], puiChannels[i]);

		/*	enable output compare preload	*/
		LL_TIM_OC_EnablePreload(pxPortTimArr[ucUnitNumber], puiChannels[i]);

		/*	select output compare mode to PWM2	*/
		LL_TIM_OC_SetMode(	pxPortTimArr[ucUnitNumber],
							puiChannels[i],
							LL_TIM_OCMODE_PWM2	);
	}

	/*	load CCR of all channels with one	*/
	LL_TIM_OC_SetCompareCH1(pxPortTimArr[ucUnitNumber], 1);
	LL_TIM_OC_SetCompareCH2(pxPortTimArr[ucUnitNumber], 1);
	LL_TIM_OC_SetCompareCH3(pxPortTimArr[ucUnitNumber], 1);
	LL_TIM_OC_SetCompareCH4(pxPortTimArr[ucUnitNumber], 1);

	/*	load repetition counter with zero	*/
	LL_TIM_SetRepetitionCounter(pxPortTimArr[ucUnitNumber], 0);

	/*	if the selected timer unit is an advanced timer (not GP timer), enable outputs	*/
	if (ucUnitNumber == 0)
	{
		LL_TIM_EnableAllOutputs(pxPortTimArr[ucUnitNumber]);
	}
}

void vPort_TIM_generateOnePulse(	uint8_t ucUnitNumber,
									uint8_t ucChannelNumber,
									uint32_t uiTimeNanoSeconds)
{
	/*	get internal clock source frequency	*/
	uint32_t uiClkInt = uiPORT_CLOCK_MAIN_HZ;

	/*	get timer tick frequency	*/
	uint64_t ulClkTick = uiClkInt / LL_TIM_GetPrescaler(pxPortTimArr[ucUnitNumber]);

	/*	Calculate number of ticks (N = time * F)	*/
	volatile uint16_t usNumberOfTicks = ((uint64_t)uiTimeNanoSeconds * ulClkTick) / 1000000000;

	/*	Number of ticks must be at least 1	*/
	if (usNumberOfTicks == 0)
		usNumberOfTicks = 1;

	/*	Load ARR with this number of ticks	*/
	LL_TIM_SetAutoReload(pxPortTimArr[ucUnitNumber], usNumberOfTicks);

	/*	load TCNT with zero	*/
	LL_TIM_SetCounter(pxPortTimArr[ucUnitNumber], 0);

	/*	Enable counter	*/
	vPORT_TIM_ENABLE_COUNTER(ucUnitNumber);
}

void vPort_TIM_enableTriggerOutput(uint8_t ucUnitNumber)
{
	LL_TIM_SetTriggerOutput(pxPortTimArr[ucUnitNumber], LL_TIM_TRGO_UPDATE);
}


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
	__asm volatile( "dsb" ::: "memory" );
	__asm volatile( "isb" );
}

void TIM1_CC_IRQHandler(void)
{
	ppfPortTimerCompareCallbackArr[0](ppvPortTimerCompareCallbackParamsArr[0]);
	vPORT_TIM_CLEAR_CC_FLAG(0);
	__asm volatile( "dsb" ::: "memory" );
	__asm volatile( "isb" );
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
	__asm volatile( "dsb" ::: "memory" );
	__asm volatile( "isb" );
#undef UNIT_NUM
}

void TIM3_IRQHandler(void)
{
#define UNIT_NUM	2

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
	__asm volatile( "dsb" ::: "memory" );
	__asm volatile( "isb" );
#undef UNIT_NUM
}

void TIM4_IRQHandler(void)
{
#define UNIT_NUM	3

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
	__asm volatile( "dsb" ::: "memory" );
	__asm volatile( "isb" );
#undef UNIT_NUM
}

#endif






#endif /* Target checking */
