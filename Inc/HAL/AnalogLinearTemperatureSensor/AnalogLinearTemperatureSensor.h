/*
 * AnalogLinearTemperatureSensor.h
 *
 *  Created on: Dec 28, 2023
 *      Author: Ali Emad
 */

#ifndef COTS_OS_INC_HAL_ANALOGLINEARTEMPERATURESENSOR_ANALOGLINEARTEMPERATURESENSOR_H_
#define COTS_OS_INC_HAL_ANALOGLINEARTEMPERATURESENSOR_ANALOGLINEARTEMPERATURESENSOR_H_


typedef struct{
	/*
	 * Number of the ADC unit that should be used when reading this sensor.
	 * (Must be previously initialized, and ADC SW driver must be previously initialized
	 * as well)
	 */
	uint8_t ucAdcUnitNumber;

	/*	Number of analog channel on which this sensor's output is connected	*/
	uint8_t ucAdcChannelNumber;

	/*
	 * Parameters of the line equation. Let the equation be:
	 * 		-	Temperature_in_mill_C = (A * Sensor_voltage_in_micro_V + B) / C
	 */
	int32_t iA;
	int32_t iB;
	int32_t iC;
}xHOS_AnalogLinearTemperatureSensor_t;

/*
 * Initializes handle of AnalogLinearTemperatureSensor sensor.
 * Notes:
 * 		-	"uiSampleTime": Is ADC sample time in micro-second, multiplied by ten.
 */
void vHOS_AnalogLinearTemperatureSensor_init(
		xHOS_AnalogLinearTemperatureSensor_t* pxHandle, uint32_t uiSampleTime	);

/*
 * Returns measured temperature in milli-celisius.
 *
 * Notes:
 * 		-	This function internally takes mutex of handle's ADC unit and gives
 * 			it back on return,
 *
 * 		-	Thus, this function is blocking for the calling task, better be called
 * 			in separate task to avoid blocking other tasks.
 *
 * 		-	This function is not ISR-safe.
 *
 * 		-	This function changes ADC channel selection. Thus it is recommended
 * 			that all ADC dependent SW select channel before reading.
 */
int32_t iHOS_AnalogLinearTemperatureSensor_getTemperature(
		xHOS_AnalogLinearTemperatureSensor_t* pxHandle);



#endif /* COTS_OS_INC_HAL_ANALOGLINEARTEMPERATURESENSOR_ANALOGLINEARTEMPERATURESENSOR_H_ */
