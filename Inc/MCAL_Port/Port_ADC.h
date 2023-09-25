/*
 * Port_ADC.h
 *
 *  Created on: ٢٤‏/٠٦‏/٢٠٢٣
 *      Author: Mohamed Mahomud Abdo
 */

#ifndef PORT_PORT_ADC_H_
#define PORT_PORT_ADC_H_

/* public */

#include "stm32f1xx.h"
#include "stm32f1xx_hal_adc.h"
#include "stm32f1xx_ll_adc.h"

extern ADC_TypeDef* const pxPortADCArr[];

/*******************************************************************************
 * ADC_TimeSampling_t.
 * What is it ?:
 * 		-	it is define the Time of the Sample
 * 		-	i only define the max and the min but you can define any number of samples
 *
 ******************************************************************************/
typedef enum{
	eSampleTimMin = ADC_SAMPLETIME_1CYCLE_5,
	eSampleTimMax = ADC_SAMPLETIME_239CYCLES_5,
}ADC_TimeSampling_t;


/*******************************************************************************
 * vPort_ADC_Init.
 * Purpose:
 * 		-	configure the ADC Parameters
 * 		-	The u16Port_ADC_PollingRead must do one converge in the call so no sequence reading
 ******************************************************************************/
void  vPort_ADC_Init(uint8_t ucADCNumber);

/*******************************************************************************
 * vPort_ADC_InitChannel.
 * Purpose:
 * 		- init the Pin for analog read
 *******************************************************************************/
void vPort_ADC_InitChannel(uint8_t ucADCNumber,uint16_t usChannelNumber,ADC_TimeSampling_t xSamplingTime);


/*******************************************************************************
 * u16Port_ADC_PollingRead.
 * Purpose:
 * 		- return the ADC conversion
 * 		- it is a blocking function
 * 		- timeout define the max time in the function
 *
 * return:
 * 		- the analog value on the pin the (12 bit res)
 *******************************************************************************/
uint16_t usPort_ADC_PollingRead(uint8_t ucAdcNumbe,uint32_t ulTimeout);

/*******************************************************************************
 * usPort_ADC_DirectRead.
 * Purpose:
 * 		- return the ADC conversion
 * 		- it is not a blocking function, just returns whatever value is in the
 * 		  ADC data register.
 *
 * return:
 * 		- the analog value on the pin the (12 bit res)
 *******************************************************************************/

#define usPORT_ADC_DIRECT_READ(ucAdcNumber)	\
	(	LL_ADC_REG_ReadConversionData12(pxPortADCArr[(ucAdcNumber)])	)

/*	Returns voltage in mV. No blocking	*/
#define iPORT_ADC_DIRECT_READ_VOLTAGE(ucAdcNumber)	\
	(	(LL_ADC_REG_ReadConversionData12(pxPortADCArr[(ucAdcNumber)]) * 3300) / 4096	)

#define vPORT_ADC_SW_TRIGGER(ucAdcNumber)	\
	(	LL_ADC_REG_StartConversionSWStart(pxPortADCArr[(ucAdcNumber)])	)



/* Error Codes */
#define Vport_ADC_initError				0
#define Vport_ADC_ChannelInitError		1
#define Vport_ADC_ReadError				2





#endif /* PORT_PORT_ADC_H_ */
