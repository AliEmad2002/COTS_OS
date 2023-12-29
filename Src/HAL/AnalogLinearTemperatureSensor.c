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

uint32_t uiHOS_AnalogLinearTemperatureSensor_getTemperature(
		xHOS_AnalogLinearTemperatureSensor_t* pxHandle)
{
	/*	Lock mutex of ADC unit	*/
	ucHOS_ADC_lockUnit(pxHandle->ucAdcUnitNumber, portMAX_DELAY);

	/*	Select channel connected to the sensor as ADC input	*/
	vHOS_ADC_selectChannel(pxHandle->ucAdcUnitNumber, pxHandle->ucAdcChannelNumber);

	/*	Start ADC conversion	*/
	vHOS_ADC_triggerRead(pxHandle->ucAdcUnitNumber);

	/*	Block until EOC	*/
	ucHOS_ADC_blockUntilEOC(pxHandle->ucAdcUnitNumber, portMAX_DELAY);

	/*	Read value	*/
	uint32_t uiRawReading = uiHOS_ADC_read(pxHandle->ucAdcUnitNumber);

	/*	Un-lock mutex of ADC unit	*/
	vHOS_ADC_unlockUnit(pxHandle->ucAdcUnitNumber);

	/*	If calibration is enabled in "AnalogLinearTemperatureSensor_Config.h"	*/
#if ucHOS_CONF_ANALOG_LINEAR_TEMPERATURE_SENSOR_ENABLE_CALIB_ADC == 1
	/*	Calibrate	*/
	uint32_t uiVrefIntRead = uiHOS_ADC_getReadingAtVrefInt();

	int32_t iVoltageUv = uiHOS_ADC_getVoltageCalib(uiRawReading, uiVrefIntRead);

	/*	Otherwise, if calibration is disabled in "AnalogLinearTemperatureSensor_Config.h"	*/
#else
	uint32_t uiVoltageUv = uiHOS_ADC_getVoltageDirect(uiRawReading);
#endif

	/*	Calculate temperature	*/
	uint32_t uiTemperatureMilliC = pxHandle->iA * iVoltageUv + pxHandle->iB;

	return uiTemperatureMilliC;
}

