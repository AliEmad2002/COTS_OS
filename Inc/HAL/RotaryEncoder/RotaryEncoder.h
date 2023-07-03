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

typedef struct{
	/*		PRIVATE		*/
	StackType_t puxTaskStack[configMINIMAL_STACK_SIZE];
	StaticTask_t xTaskStatic;
	TaskHandle_t xTask;

	TickType_t xLastActiveTimeStamp;

	int8_t cCurrentDirection;

	int32_t iCount;

	uint8_t ucAPrevLevel;
	uint8_t ucBPrevLevel;

	uint8_t ucFirstEdgeChannel;

	/*		PUBLIC		*/
	uint8_t ucAPort;
	uint8_t ucAPin;

	uint8_t ucBPort;
	uint8_t ucBPin;

	/*
	 * Any of the following can be changed while object is active and running,
	 * change effects would take place next sample.
	 */
	uint32_t uiSamplePeriodMs;

	uint32_t uiIdleTimeoutMs;
}xHOS_RotaryEncoder_t;

void vHOS_RotaryEncoder_init(xHOS_RotaryEncoder_t* pxHandle);

int32_t iHOS_RotaryEncoder_getCount(xHOS_RotaryEncoder_t* pxHandle);

#endif /* HAL_OS_INC_ROTARYENCODER_ROTARYENCODER_H_ */
