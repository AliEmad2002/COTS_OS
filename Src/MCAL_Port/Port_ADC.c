/*
 * Port_ADC.c
 *
 *  Created on: ٢٤‏/٠٦‏/٢٠٢٣
 *      Author: mohamed
 */

/*	LIB	*/
#include "stdint.h"

/* Hal */

#include "stm32f1xx.h"
#include "stm32f1xx_hal_adc.h"
#include "stm32f1xx_ll_adc.h"

/* Port */
#include "MCAL_Port/Port_ADC.h"
#include "MCAL_Port/Port_GPIO.h"
#include "MCAL_Port/Port_Interrupt.h"

/* Errors */
#include "LIB/Assert.h"

/* private */
ADC_TypeDef* const pxPortADCArr[] = {ADC1, ADC2};

/* public */

void  vPort_ADC_Init(uint8_t ucADCNumber)
{
	ADC_HandleTypeDef hadc = {0};

	hadc.Init.ContinuousConvMode = DISABLE;
	hadc.Init.DiscontinuousConvMode = DISABLE;
	hadc.Init.NbrOfConversion = 1;
	hadc.Init.ScanConvMode = DISABLE;
	hadc.Init.DataAlign  = ADC_DATAALIGN_RIGHT;
	hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	hadc.Instance = pxPortADCArr[ucADCNumber];

	if (HAL_ADC_Init(&hadc) != HAL_OK)
	{ /* Initialization Error */
		vLib_ASSERT(0,Vport_ADC_initError);
	}

	ADC_Enable(&hadc);
}

void vPort_ADC_InitChannel(uint8_t ucADCNumber,uint16_t usChannelNumber,ADC_TimeSampling_t xSamplingTime)
{
	/*	TODO: use LL driver as it is less resource consuming	*/
	ADC_HandleTypeDef hadc = {0};

	hadc.Instance = pxPortADCArr[ucADCNumber];

	ADC_ChannelConfTypeDef sConfig;
	sConfig.Channel = usChannelNumber;
	sConfig.Rank = 1;
	sConfig.SamplingTime = xSamplingTime;


	if ( 	HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
	{ /* Initialization Error */

		vLib_ASSERT(0,Vport_ADC_ChannelInitError);

	}

}


/*******************************************************************************
 * Static variables:
 ******************************************************************************/
#ifdef ucPORT_INTERRUPT_IRQ_DEF_ADC

void (*ppfPortAdcIsrCallback[2])(void*);
void* ppvPortAdcIsrParams[2];

#endif	/*	ucPORT_INTERRUPT_IRQ_DEF_ADC	*/


/*******************************************************************************
 * API functions:
 ******************************************************************************/
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

void (*ppfPortAdcIsrCallback[2])(void*);
void* ppvPortAdcIsrParams[2];

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

