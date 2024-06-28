/*
 * Port_ADC.c
 *
 *  Created on: 2023/12/11
 *      Author: Ali Emad
 */

/*	Target checking	*/
#include "MCAL_Port/Port_Target.h"
#ifdef MCAL_PORT_TARGET_STM32F401RCT6


/*	LIB	*/
#include "stdint.h"
#include "LIB/Assert.h"

/* MCAL */
#include "stm32f4xx.h"
#include "stm32f4xx_hal_adc.h"
#include "stm32f4xx_ll_adc.h"

#include "MCAL_Port/Port_Clock.h"
#include "MCAL_Port/Port_ADC.h"
#include "MCAL_Port/Port_GPIO.h"
#include "MCAL_Port/Port_Interrupt.h"


/*******************************************************************************
 * Driver variables:
 ******************************************************************************/
ADC_TypeDef* const pxPortADCArr[] = {ADC1};

const uint32_t pxPortADCChannelsArr[] = {
		LL_ADC_CHANNEL_0,
		LL_ADC_CHANNEL_1,
		LL_ADC_CHANNEL_2,
		LL_ADC_CHANNEL_3,
		LL_ADC_CHANNEL_4,
		LL_ADC_CHANNEL_5,
		LL_ADC_CHANNEL_6,
		LL_ADC_CHANNEL_7,
		LL_ADC_CHANNEL_8,
		LL_ADC_CHANNEL_9,
		LL_ADC_CHANNEL_10,
		LL_ADC_CHANNEL_11,
		LL_ADC_CHANNEL_12,
		LL_ADC_CHANNEL_13,
		LL_ADC_CHANNEL_14,
		LL_ADC_CHANNEL_15,
		LL_ADC_CHANNEL_16,
		LL_ADC_CHANNEL_17,
		LL_ADC_CHANNEL_18
};

#ifdef ucPORT_INTERRUPT_IRQ_DEF_ADC
	void (*ppfPortAdcIsrCallback[1])(void*);
	void* ppvPortAdcIsrParams[1];
#endif	/*	ucPORT_INTERRUPT_IRQ_DEF_ADC	*/

/*******************************************************************************
 * Macros:
 ******************************************************************************/
#define uiADC_CLOCK_HZ		(uiPORT_CLOCK_MAIN_HZ / uiPORT_CLOCK_ADC_DIV)

/*******************************************************************************
 * Static (private) functions:
 ******************************************************************************/
void vADC_selectChannel(uint8_t ucUnitNumber, uint8_t ucChannelNumber)
{
	if (ucChannelNumber == ucPORT_ADC_BKP_BAT_CH_NUMBER)
	{
		LL_ADC_SetCommonPathInternalCh(
				__LL_ADC_COMMON_INSTANCE(pxPortADCArr[0]),
				LL_ADC_PATH_INTERNAL_VREFINT | LL_ADC_PATH_INTERNAL_VBAT	);
	}

	else if (ucChannelNumber == ucPORT_ADC_TEMP_SENS_INT_CH_NUMBER)
	{
		LL_ADC_SetCommonPathInternalCh(
				__LL_ADC_COMMON_INSTANCE(pxPortADCArr[0]),
				LL_ADC_PATH_INTERNAL_VREFINT | LL_ADC_PATH_INTERNAL_TEMPSENSOR	);

		ucChannelNumber = 18;
	}

	LL_ADC_REG_SetSequencerRanks(
			pxPortADCArr[(ucUnitNumber)],
			LL_ADC_REG_RANK_1,
			pxPortADCChannelsArr[(ucChannelNumber)]	);
}

/*******************************************************************************
 * API functions:
 ******************************************************************************/
/*
 * See header for info.
 */
void vPort_ADC_init(uint8_t ucUnitNumber)
{
	ADC_HandleTypeDef xAdcHandle = {0};

	xAdcHandle.Init.ContinuousConvMode = DISABLE;
	xAdcHandle.Init.DiscontinuousConvMode = DISABLE;
	xAdcHandle.Init.NbrOfConversion = 1;
	xAdcHandle.Init.ScanConvMode = DISABLE;
	xAdcHandle.Init.DataAlign  = ADC_DATAALIGN_RIGHT;
	xAdcHandle.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	xAdcHandle.Instance = pxPortADCArr[ucUnitNumber];

	vLib_ASSERT(HAL_ADC_Init(&xAdcHandle)==HAL_OK, 0);

	__HAL_ADC_ENABLE(&xAdcHandle);

	LL_ADC_SetCommonPathInternalCh(
			__LL_ADC_COMMON_INSTANCE(pxPortADCArr[0]), LL_ADC_PATH_INTERNAL_VREFINT);

	vPort_ADC_setChannelSampleTime(0, ucPORT_ADC_TEMP_SENS_INT_CH_NUMBER, 171);	/*	Internal temperature sensor	*/
	vPort_ADC_setChannelSampleTime(0, ucPORT_ADC_VREFINT_CH_NUMBER, 171);	/*	Internal voltage reference	*/
	vPort_ADC_setChannelSampleTime(0, ucPORT_ADC_BKP_BAT_CH_NUMBER, 171);	/*	Internal voltage reference	*/
}

/*
 * See header for info.
 */
void vPort_ADC_setChannelSampleTime(	uint8_t ucUnitNumber,
										uint8_t ucChannelNumber,
										uint32_t uiSampleTimeInUsBy10	)
{
	/*
	 * TODO: make a macro to evaluate the following calculation. It doesn't have
	 * to be in runtime.
	 */

	/*
	 * Get how many ADC clock cycles are needed to achieve the requested sample
	 * time.
	 */
	uint32_t uiNCyclesBy10 =
			((uint64_t)uiSampleTimeInUsBy10 * (uint64_t)uiADC_CLOCK_HZ) / 1000000;

	/*
	 * Select the closest possible option.
	 */
	uint32_t uiOption;
	if 		(uiNCyclesBy10 < 30+(150-30)/2)			uiOption = ADC_SAMPLETIME_3CYCLES  ;
	else if (uiNCyclesBy10 < 150+(280-150)/2)		uiOption = ADC_SAMPLETIME_15CYCLES ;
	else if (uiNCyclesBy10 < 280+(560-280)/2)		uiOption = ADC_SAMPLETIME_28CYCLES ;
	else if (uiNCyclesBy10 < 560+(840-560)/2)		uiOption = ADC_SAMPLETIME_56CYCLES ;
	else if (uiNCyclesBy10 < 840+(1120-840)/2)		uiOption = ADC_SAMPLETIME_84CYCLES ;
	else if (uiNCyclesBy10 < 1120+(1440-1120)/2)	uiOption = ADC_SAMPLETIME_112CYCLES;
	else if (uiNCyclesBy10 < 1440+(4800-1440)/2)	uiOption = ADC_SAMPLETIME_144CYCLES;
	else 											uiOption = ADC_SAMPLETIME_480CYCLES;

	/*	Set that option	*/
	LL_ADC_SetChannelSamplingTime(
			pxPortADCArr[ucUnitNumber], pxPortADCChannelsArr[ucChannelNumber], uiOption	);
}

void vPort_ADC_setInterruptCallback(	uint8_t ucUnitNumber,
								void(*pfCallback)(void*),
								void* pvParams	)
{
	ppfPortAdcIsrCallback[ucUnitNumber] = pfCallback;
	ppvPortAdcIsrParams[ucUnitNumber] = pvParams;
}

void vPort_ADC_setConversionMode(uint8_t ucUnitNumber, uint8_t ucMode)
{
	if (ucMode == 0)
		LL_ADC_REG_SetContinuousMode(pxPortADCArr[ucUnitNumber], LL_ADC_REG_CONV_CONTINUOUS);
	else
		LL_ADC_REG_SetContinuousMode(pxPortADCArr[ucUnitNumber], LL_ADC_REG_CONV_SINGLE);
}


/*******************************************************************************
 * ISRs:
 ******************************************************************************/
#ifdef ucPORT_INTERRUPT_IRQ_DEF_ADC

void ADC_IRQHandler(void)
{
	/*
	 * Important!:
	 * When viewing ADC_DR in debugger, EOC flag  will be cleared immediately, and
	 * hence callback selection will not occur properly.
	 * (As this flag is cleared on DR read)
	 */

	if (ucPORT_ADC_GET_EOC_FLAG(0))
	{
		ppfPortAdcIsrCallback[0](ppvPortAdcIsrParams[0]);
		vPORT_ADC_CLR_EOC_FLAG(0);
	}

	else if (ucPORT_ADC_GET_EOC_FLAG(1))
	{
		ppfPortAdcIsrCallback[1](ppvPortAdcIsrParams[1]);
		vPORT_ADC_CLR_EOC_FLAG(1);
	}
}

#endif	/*	ucPORT_INTERRUPT_IRQ_DEF_ADC	*/



#endif /* Target checking */
