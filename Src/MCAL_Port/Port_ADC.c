/*
 * Port_ADC.c
 *
 *  Created on: 2023/12/11
 *      Author: Ali Emad
 */

/*	LIB	*/
#include "stdint.h"
#include "LIB/Assert.h"

/* MCAL */
#include "stm32f1xx.h"
#include "stm32f1xx_hal_adc.h"
#include "stm32f1xx_ll_adc.h"

#include "MCAL_Port/Port_Clock.h"
#include "MCAL_Port/Port_ADC.h"
#include "MCAL_Port/Port_GPIO.h"
#include "MCAL_Port/Port_Interrupt.h"


/*******************************************************************************
 * Driver variables:
 ******************************************************************************/
ADC_TypeDef* const pxPortADCArr[] = {ADC1, ADC2};

#ifdef ucPORT_INTERRUPT_IRQ_DEF_ADC
	void (*ppfPortAdcIsrCallback[2])(void*);
	void* ppvPortAdcIsrParams[2];
#endif	/*	ucPORT_INTERRUPT_IRQ_DEF_ADC	*/

/*******************************************************************************
 * Macros:
 ******************************************************************************/
#define uiADC_CLOCK_HZ		(uiPORT_CLOCK_MAIN_HZ / uiPORT_CLOCK_ADC_DIV)

/*******************************************************************************
 * Static (private) functions:
 ******************************************************************************/


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

	ADC_Enable(&xAdcHandle);

	HAL_ADCEx_Calibration_Start(&xAdcHandle);

	vPort_ADC_setChannelSampleTime(0, 16, 171);	/*	Internal temperature sensor	*/
	vPort_ADC_setChannelSampleTime(0, 17, 171);	/*	Internal voltage reference	*/
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
	if 		(uiNCyclesBy10 < 15+(75-15)/2)		uiOption = ADC_SAMPLETIME_1CYCLE_5;
	else if (uiNCyclesBy10 < 75+(135-75)/2)		uiOption = ADC_SAMPLETIME_7CYCLES_5;
	else if (uiNCyclesBy10 < 135+(285-135)/2)	uiOption = ADC_SAMPLETIME_13CYCLES_5;
	else if (uiNCyclesBy10 < 285+(415-285)/2)	uiOption = ADC_SAMPLETIME_28CYCLES_5;
	else if (uiNCyclesBy10 < 415+(555-415)/2)	uiOption = ADC_SAMPLETIME_41CYCLES_5;
	else if (uiNCyclesBy10 < 555+(715-555)/2)	uiOption = ADC_SAMPLETIME_55CYCLES_5;
	else if (uiNCyclesBy10 < 715+(2395-715)/2)	uiOption = ADC_SAMPLETIME_71CYCLES_5;
	else 										uiOption = ADC_SAMPLETIME_239CYCLES_5;

	/*	Set that option	*/
	LL_ADC_SetChannelSamplingTime(
			pxPortADCArr[ucUnitNumber], LL_ADC_CHANNEL_17/*	TODO	*/, uiOption	);
}

void vPort_ADC_setInterruptCallback(	uint8_t ucUnitNumber,
								void(*pfCallback)(void*),
								void* pvParams	)
{
	ppfPortAdcIsrCallback[ucUnitNumber] = pfCallback;
	ppvPortAdcIsrParams[ucUnitNumber] = pvParams;
}


/*******************************************************************************
 * ISRs:
 ******************************************************************************/
#ifdef ucPORT_INTERRUPT_IRQ_DEF_ADC

void ADC1_2_IRQHandler(void)
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

