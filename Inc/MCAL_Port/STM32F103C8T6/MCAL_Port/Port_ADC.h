/*
 * Port_ADC.h
 *
 *  Created on: 2023/12/11
 *      Author: Ali Emad
 */

/*	Target checking	*/
#include "MCAL_Port/Port_Target.h"
#ifdef MCAL_PORT_TARGET_STM32F103C8T6


#ifndef PORT_PORT_ADC_H_
#define PORT_PORT_ADC_H_


#include "stm32f1xx.h"
#include "stm32f1xx_hal_adc.h"
#include "stm32f1xx_ll_adc.h"

extern ADC_TypeDef* const pxPortADCArr[];
extern const uint32_t pxPortADCChannelsArr[];

/*******************************************************************************
 * Defining target dependent values (Needed by upper layers):
 ******************************************************************************/
#define ucPORT_ADC_IS_VREF_INT_AVAILABLE		1

#if ucPORT_ADC_IS_VREF_INT_AVAILABLE
	/*	Number of unit and channel connected to internal voltage reference	*/
	#define ucPORT_ADC_VREFINT_UNIT_NUMBER		(	0	)
	#define ucPORT_ADC_VREFINT_CH_NUMBER		(	17	)

	/*	Value of internal voltage reference in mV	*/
	#define uiPORT_ADC_VREFINT_IN_MV			(	1200	)
#endif	/*	ucPORT_ADC_IS_VREF_INT_AVAILABLE	*/

/*	Value of voltage reference in mV	*/
#define uiPORT_ADC_VREF_IN_MV				(	3300	)

/*	Number of unit and channel connected to internal temperature sensor	*/
#define ucPORT_ADC_TEMP_SENS_INT_UNIT_NUMBER		(	0	)
#define ucPORT_ADC_TEMP_SENS_INT_CH_NUMBER			(	16	)

/*
 * Internal temperature sensor equation parameters, such that:
 * 		-	Temperature_in_mill_C = (A * Sensor_voltage_in_micro_V + B) / C
 */
#define iPORT_ADC_TEMP_SENS_A						(	-10			)
#define iPORT_ADC_TEMP_SENS_B						(	15075000	)
#define iPORT_ADC_TEMP_SENS_C						(	43			)

/*
 * Number of timer unit used as trigger source for the ADC unit. (Used by upper
 * layer SW when there's a need o synchronize ADC sampling).
 *
 */
extern const uint8_t pucPortADCTriggeringTimerUnitNumber[];

/*******************************************************************************
 * API functions / macros:
 ******************************************************************************/
/*	Initializes unit	*/
void vPort_ADC_init(uint8_t ucUnitNumber);

/*
 * Sets channel's sample time to the nearest possible.
 *
 * Notes:
 * 		-	"uiSampleTimeInUsBy10": is the wanted sample time in us, multiplied
 * 			by 10.
 *
 * 		-	Remember that sample time and conversion time are not the same thing.
 * 			Refer to datasheet for more info.
 */
void vPort_ADC_setChannelSampleTime(	uint8_t ucUnitNumber,
										uint8_t ucChannelNumber,
										uint32_t uiSampleTimeInUsBy10	);

/*	Selects channel to be used by a unit	*/
#define vPORT_ADC_SELECT_CHANNEL(ucUnitNumber, ucChannelNumber)	\
	(LL_ADC_REG_SetSequencerRanks(pxPortADCArr[(ucUnitNumber)], LL_ADC_REG_RANK_1, pxPortADCChannelsArr[(ucChannelNumber)]))

/*	Reads EOC flag.	*/
#define ucPORT_ADC_GET_EOC_FLAG(ucUnitNumber)	\
	(LL_ADC_IsActiveFlag_EOS(pxPortADCArr[(ucUnitNumber)]))

/*	Clears EOC flag.	*/
#define vPORT_ADC_CLR_EOC_FLAG(ucUnitNumber)	\
	(LL_ADC_ClearFlag_EOS(pxPortADCArr[(ucUnitNumber)]))

/*	Reads start flag.	*/
#define ucPORT_ADC_GET_START_FLAG(ucUnitNumber)	\
	(LL_ADC_IsActiveFlag_s(pxPortADCArr[(ucUnitNumber)]))

/*	Enables EOC interrupt	*/
#define vPORT_ADC_ENABLE_EOC_INTERRUPT(ucUnitNumber)	\
	(LL_ADC_EnableIT_EOS(pxPortADCArr[(ucUnitNumber)]))

/*	Disables EOC interrupt	*/
#define vPORT_ADC_DISABLE_EOC_INTERRUPT(ucUnitNumber)	\
	(LL_ADC_DisableIT_EOS(pxPortADCArr[(ucUnitNumber)]))

/*	Sets EOC ISR callback	*/
void vPort_ADC_setInterruptCallback(	uint8_t ucUnitNumber,
										void(*pfCallback)(void*),
										void* pvParams	);

/*	Triggers conversion start	*/
#define vPORT_ADC_TRIGGER_CONVERSION(ucUnitNumber)	\
	(LL_ADC_REG_StartConversionSWStart(pxPortADCArr[(ucUnitNumber)]))

/*	Gets DR value (latest completed conversion)	*/
#define usPORT_ADC_GET_DR(ucAdcNumber)	\
	(	LL_ADC_REG_ReadConversionData12(pxPortADCArr[(ucAdcNumber)])	)

/*
 * Selects conversion triggering source (SW or timer)
 *
 * Notes:
 * 		-	"ucSrc": 0==> SW trigger,	1==> Timer trigger.
 * 		-	Timer unit is configured in "pucPortADCTriggeringTimerUnitNumber[]".
 */
void vPort_ADC_setTriggerSource(uint8_t ucUnitNumber, uint8_t ucSrc);

/*
 * Selects conversion mode.
 *
 * Notes:
 * 		-	"ucMode": 0 ==> Continuous, 1 ==> Single sample.
 */
void vPort_ADC_setConversionMode(uint8_t ucUnitNumber, uint8_t ucMode);











#endif /* PORT_PORT_ADC_H_ */


#endif /* Target checking */
