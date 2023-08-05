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

/*******************************************************************************
 * Structures:
 ******************************************************************************/
typedef struct{
	/*		PRIVATE		*/
	StackType_t puxTaskStack[configMINIMAL_STACK_SIZE];
	StaticTask_t xTaskStatic;
	TaskHandle_t xTask;

	TickType_t xLastActiveTimeStamp;

	uint8_t ucAPrevLevel;
	uint8_t ucBPrevLevel;

	uint8_t ucANewLevel;
	uint8_t ucBNewLevel;

	uint8_t ucALevelFiltered;
	uint8_t ucBLevelFiltered;

	uint8_t ucAPrevLevelFiltered;
	uint8_t ucBPrevLevelFiltered;

	uint8_t ucNA;
	uint8_t ucNB;

	/*		PUBLIC		*/
	uint8_t ucAPort;
	uint8_t ucAPin;

	uint8_t ucBPort;
	uint8_t ucBPin;

	/*
	 * Any of the following can be changed while object is active and running,
	 * change effects would take place next sample.
	 */
	uint32_t uiSamplePeriodMs; // recommended: 5ms

	uint8_t ucNFilter; // recommended: 10 / uiSamplePeriodMs

	int32_t iPos;

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
void vHOS_RotaryEncoder_init(xHOS_RotaryEncoder_t* pxHandle);

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
