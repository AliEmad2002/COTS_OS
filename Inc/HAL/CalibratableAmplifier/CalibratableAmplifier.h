/*
 * CalibratableAmplifier.h
 *
 *  Created on: Dec 29, 2023
 *      Author: Ali Emad
 *
 * This software interfaces external amplifier circuits which are calibratable.
 * An example circuit is shown in "CalibratableAmplifier/example.png"
 */

#ifndef COTS_OS_INC_HAL_CALIBRATABLEAMPLIFIER_CALIBRATABLEAMPLIFIER_H_
#define COTS_OS_INC_HAL_CALIBRATABLEAMPLIFIER_CALIBRATABLEAMPLIFIER_H_

#define uiHOS_CALIBRATABLEAMPLIFIER_HANDLE_TANSK_STACK_SIZE		(128)

typedef struct{
	/*		PUBLIC		*/
	/*	ADC connection	*/
	uint8_t ucAdcUnitNumber;
	uint8_t ucAdcChannelNumber;

	/*	Amplifier gain in [uV/V]	*/
	int32_t iGainUVPV;

	/*
	 * Control pins.
	 * As number of control pins differ from one amplifier circuit to the other,
	 * an array of pins is user defined and assigned by reference to the following
	 * pointers.
	 */
	uint8_t* pucCtrlPortArr;
	uint8_t* pucCtrlPinArr;
	uint8_t ucNumberOfCtrlPins;

	/*
	 * States of control pins when reading actual amplifier output.
	 * ucAmpOnCtrlStates = (state0 << 0) | (state1 << 1) | ...
	 *
	 * state_i is calculated as follows:
	 * 		if (HighStateMask & (1 << i)):
	 * 			state_i = high
	 * 		else if (LowStateMask & (1 << i)):
	 * 			state_i = low
	 * 		else
	 *	 		state_i = disconnected
	 */
	uint8_t ucAmpOnCtrlHighStateMask;
	uint8_t ucAmpOnCtrlLowStateMask;

	/*
	 * States of control pins when reading amplifier offset.
	 */
	uint8_t ucAmpCalibCtrlHighStateMask;
	uint8_t ucAmpCalibCtrlLowStateMask;

	/*	Indicates whether initial calibration is done or not (Read only)	*/
	uint8_t ucIsInitialCalibDone;

	/*	Time period in ms, in which the software calibrates the amplifier circuit	*/
	uint32_t uiCalibTimePeriodMs;

	/*
	 * Time since switching control signals, until circuit is of stable state,
	 * in ms.
	 */
	uint32_t uiSwitchTimeMs;

	/*	Time window at which samples are averaged when calibrating	*/
	uint32_t uiCalibAvgTimeMs;

	/*		PRIVATE		*/
	/*	Offset value (ADC raw output. Not voltage)	*/
	uint32_t uiOffsetRaw;

	/*	Mutex of the amplifier circuit	*/
	SemaphoreHandle_t xMutex;
	StaticSemaphore_t xMutexStatic;

	/*	Task that periodically calibrates the amplifier circuit	*/
	TaskHandle_t xTask;
	StaticTask_t xTaskStatic;
	StackType_t pxStack[uiHOS_CALIBRATABLEAMPLIFIER_HANDLE_TANSK_STACK_SIZE];
}xHOS_CalibratableAmplifier_t;

/*
 * Initializes handle.
 *
 * Notes:
 * 		-	All public parameters of the handle must be previously initialized.
 *
 * 		-	"uiSampleTime": Is sample time in micro-second, multiplied by ten.
 */
void vHOS_CalibratableAmplifier_init(	xHOS_CalibratableAmplifier_t* pxHandle,
										uint32_t uiSampleTime	);

/*
 * Reads voltage measured by the amplifier. Return value is in uV.
 *
 * Notes:
 * 		-	This function internally locks and then unlocks mutex of the handle.
 *
 * 		-	If handle's mutex was not acquired in the given timeout, function returns
 * 			MAX(int32_t) i.e.: 2^31 - 1
 *
 * 		-	Check "ucIsInitialCalibDone" is 1 before reading for the first time.
 */
int32_t iHOS_CalibratableAmplifier_read(
		xHOS_CalibratableAmplifier_t* pxHandle,
		TickType_t xTimeout	);



#endif /* COTS_OS_INC_HAL_CALIBRATABLEAMPLIFIER_CALIBRATABLEAMPLIFIER_H_ */
