/*
 * Thermocouple.h
 *
 *  Created on: Dec 31, 2023
 *      Author: Ali Emad
 */

#ifndef COTS_OS_INC_HAL_THERMOCOUPLE_THERMOCOUPLE_H_
#define COTS_OS_INC_HAL_THERMOCOUPLE_THERMOCOUPLE_H_


typedef struct{
	/*		PUBLIC		*/
	/*
	 * For more accurate results, cold junction temperature (i.e.: ambient temperaturee)
	 * should be used.
	 *
	 * Notes:
	 * 		-	"pxColdJunctionTemperatureSensor" is a pointer to a previously
	 * 			initialized sensor handle.
	 *
	 * 		-	If no sensor is used, assign "NULL" to this pointer.
	 */
	xHOS_AnalogLinearTemperatureSensor_t* pxColdJunctionTemperatureSensor;

	/*
	 * As thermocouple voltage is too low in range to be read directly, an external
	 * HW amplifier must be connected.
	 *
	 * Notes:
	 * 		-	"pxAmplifier" is a pointer to a previously initialized amplifier
	 * 			handle.
	 */
	xHOS_CalibratableAmplifier_t* pxAmplifier;

	/*		PRIVATE		*/
	uint32_t uiTLPrev;
}xHOS_Thermocouple_t;

/*	Initializes handle	*/
void vHOS_Thermocouple_init(xHOS_Thermocouple_t* pxHandle);

/*	Returns temperature in milli-C	*/
uint8_t ucHOS_Thermocouple_getTemperature(	xHOS_Thermocouple_t* pxHandle,
											int32_t* piT	);






#endif /* COTS_OS_INC_HAL_THERMOCOUPLE_THERMOCOUPLE_H_ */
