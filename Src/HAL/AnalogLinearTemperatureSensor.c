/*
 * AnalogLinearTemperatureSensor.c
 *
 *  Created on: Dec 28, 2023
 *      Author: Ali Emad
 */

/*	LIB	*/
#include "stdint.h"

/*	RTOS	*/
#include "FreeRTOS.h"
#include "semphr.h"

/*	MCAL	*/
#include "MCAL_Port/Port_ADC.h"

/*	HAL	*/
#include "HAL/ADC/ADC.h"

/*	SELF	*/
#include "HAL/AnalogLinearTemperatureSensor/AnalogLinearTemperatureSensor.h"
#include "HAL/AnalogLinearTemperatureSensor/AnalogLinearTemperatureSensor_Config.h"

void vHOS_AnalogLinearTemperatureSensor_init(
		xHOS_AnalogLinearTemperatureSensor_t* pxHandle, uint32_t uiSampleTime	)
{
	vHOS_ADC_setSampleTime(	pxHandle->ucAdcUnitNumber,
							pxHandle->ucAdcChannelNumber,
							uiSampleTime	);
}

int32_t iHOS_AnalogLinearTemperatureSensor_getTemperature(
		xHOS_AnalogLinearTemperatureSensor_t* pxHandle)
{
	/*	Lock mutex of ADC unit	*/
	ucHOS_ADC_lockUnit(pxHandle->ucAdcUnitNumber, portMAX_DELAY);

	/*	Read value	*/
	uint32_t uiRawReading = uiHOS_ADC_readChannelBlocking(
			pxHandle->ucAdcUnitNumber, pxHandle->ucAdcChannelNumber);

	/*	Un-lock mutex of ADC unit	*/
	vHOS_ADC_unlockUnit(pxHandle->ucAdcUnitNumber);

	/*	If calibration is enabled in "AnalogLinearTemperatureSensor_Config.h"	*/
#if ucHOS_CONF_ANALOG_LINEAR_TEMPERATURE_SENSOR_ENABLE_CALIB_ADC == 1
	/*	Calibrate	*/
	ucHOS_ADC_lockUnit(ucPORT_ADC_VREFINT_UNIT_NUMBER, portMAX_DELAY);

	uint32_t uiVrefIntRead = uiHOS_ADC_readChannelBlocking(
			ucPORT_ADC_VREFINT_UNIT_NUMBER, ucPORT_ADC_VREFINT_CH_NUMBER	);

	vHOS_ADC_unlockUnit(ucPORT_ADC_VREFINT_UNIT_NUMBER);

	int32_t iVoltageUv = iHOS_ADC_getVoltageCalib(uiRawReading, uiVrefIntRead);

	/*	Otherwise, if calibration is disabled in "AnalogLinearTemperatureSensor_Config.h"	*/
#else
	uint32_t uiVoltageUv = uiHOS_ADC_getVoltageDirect(uiRawReading);
#endif

	/*	Calculate temperature	*/
	int32_t iTemperatureMilliC =
			((int64_t)pxHandle->iA * (int64_t)iVoltageUv + pxHandle->iB) / pxHandle->iC;

	return iTemperatureMilliC;
}

