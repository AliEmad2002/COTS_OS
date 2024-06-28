/*
 * OneWireTemperatureSensor.h
 *
 *  Created on: Jun 22, 2024
 *      Author: Ali Emad
 *
 * Tested on: DS18B20
 */

#ifndef COTS_OS_INC_HAL_ONEWIRETEMPERATURESENSOR_ONEWIRETEMPERATURESENSOR_H_
#define COTS_OS_INC_HAL_ONEWIRETEMPERATURESENSOR_ONEWIRETEMPERATURESENSOR_H_


typedef struct{
	/*	PRIVATE	*/
	StaticSemaphore_t xMutexMem;
	SemaphoreHandle_t xMutex;

	/*	PUBLIC	*/
	xHOS_HardwareDelay_t* pxHwDelayHandle;
	uint8_t pucAdderssArr[8];
	uint8_t ucPort;
	uint8_t ucPin;
}xHOS_OneWireTemperatureSensor_t;


/*
 * Initializes handle.
 *
 * Notes:
 * 	-	All public parameters must be initialized first.
 */
void vHOS_OneWireTemperatureSensor_init(xHOS_OneWireTemperatureSensor_t* pxHandle);

/*	Locks handle	*/
uint8_t ucHOS_OneWireTemperatureSensor_lock(
	xHOS_OneWireTemperatureSensor_t* pxHandle,
	TickType_t xTimeout	);

/*	Unlocks handle	*/
void vHOS_OneWireTemperatureSensor_unlock(xHOS_OneWireTemperatureSensor_t* pxHandle);

/*
 * Reads temperature.
 *
 * Notes:
 * 	-	Function writes temperature in milli-Celsius to "piTemperature".
 * 	-	Handle must be locked by calling task.
 * 	-	If read can't be done in the given timeout, function returns 0, otherwise,
 * 		if successful, in returns 1.
 */
uint8_t ucHOS_OneWireTemperatureSensor_getTemperature(
	xHOS_OneWireTemperatureSensor_t* pxHandle,
	TickType_t xTimeout,
	int32_t* piTemperature	);



#endif /* COTS_OS_INC_HAL_ONEWIRETEMPERATURESENSOR_ONEWIRETEMPERATURESENSOR_H_ */
