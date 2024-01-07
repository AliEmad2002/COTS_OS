/*
 * RotaryEncoder.h
 *
 *  Created on: Jun 29, 2023
 *      Author: Ali Emad
 *
 * Notes:
 * 		-	This driver assumes that common pin is connected to Vcc.
 */

#ifndef HAL_OS_INC_ROTARYENCODER_ROTARYENCODER_H_
#define HAL_OS_INC_ROTARYENCODER_ROTARYENCODER_H_

#include "FreeRTOS.h"
#include "LIB/BinaryFilter/BinaryFilter.h"

#define uiHOS_ROTARYENCODER_STACK_SZ	128

/*******************************************************************************
 * Structures:
 ******************************************************************************/
typedef struct{
	/*		PRIVATE		*/
	StackType_t puxTaskStack[uiHOS_ROTARYENCODER_STACK_SZ];
	StaticTask_t xTaskStatic;
	TaskHandle_t xTask;

	/*		PUBLIC		*/
	uint8_t ucAPort;
	uint8_t ucAPin;

	uint8_t ucBPort;
	uint8_t ucBPin;

	xLIB_BinaryFilter_t xAFilter;	// read only.
	xLIB_BinaryFilter_t xBFilter;	// read only.

	/*
	 * Any of the following can be changed while object is active and running,
	 * change effects would take place next sample.
	 */
	uint32_t uiSamplePeriodMs;
	uint32_t uiSpeedUpdatePeriodMs;
	uint32_t uiSpeedDeadTimeMs; // If the position stays constant for this time,
								// speed is set to zero.

	int32_t iPos;
	int32_t iSpeed;		// read only.
	uint8_t ucEnableSpeedUpdate;

	uint8_t ucEnableCWCallback;
	void (*pfCWCallback)(void*);
	void* pvCWParams;

	uint8_t ucEnableCCWCallback;
	void (*pfCCWCallback)(void*);
	void* pvCCWParams;
}xHOS_RotaryEncoder_t;

/*******************************************************************************
 * API functions:
 ******************************************************************************/
/*
 * Initializes object and its task.
 *
 * Notes:
 * 		-	Must be called before scheduler start.
 * 		-	All public parameters of the passed handle must be initialized first.
 */
void vHOS_RotaryEncoder_init(xHOS_RotaryEncoder_t* pxHandle, uint8_t ucNFilter);

/*
 * Enables object.
 *
 * Notes:
 * 		-	A "xHOS_RotaryEncoder_t" is initially disabled.
 * 		-	This is an inline function.
 */
void vHOS_RotaryEncoder_enable(xHOS_RotaryEncoder_t* pxHandle);

/*
 * Disables object.
 *
 * Notes:
 * 		-	A "xHOS_RotaryEncoder_t" is initially disabled.
 * 		-	This is an inline function.
 */
void vHOS_RotaryEncoder_disable(xHOS_RotaryEncoder_t* pxHandle);

#endif /* HAL_OS_INC_ROTARYENCODER_ROTARYENCODER_H_ */
