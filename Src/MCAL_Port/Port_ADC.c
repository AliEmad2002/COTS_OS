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

/* Port */
#include "MCAL_Port/Port_ADC.h"
#include "MCAL_Port/Port_GPIO.h"

/* Errors */
#include "LIB/Assert.h"

/* private */
ADC_TypeDef* const pxPortADCArr[] = {ADC1, ADC2};

/* public */

void  vPort_ADC_Init(uint8_t ucADCNumber)
{
	ADC_HandleTypeDef hadc = {0};

	hadc.Init.ContinuousConvMode = ENABLE;
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

uint16_t usPort_ADC_PollingRead(uint8_t ucAdcNumbe,uint32_t ulTimeout)
{
	ADC_HandleTypeDef hadc;
	hadc.Instance = pxPortADCArr[ucAdcNumbe];
	HAL_ADC_Start(&hadc);
	while(ulTimeout > 0)
	{
		if(HAL_ADC_PollForConversion(&hadc, 0) == HAL_OK)
			{
				uint16_t adc_read = HAL_ADC_GetValue(&hadc);
				return adc_read;
			}

		ulTimeout--;
	}

	vLib_ASSERT(0, Vport_ADC_ReadError);
	return 0;

}
