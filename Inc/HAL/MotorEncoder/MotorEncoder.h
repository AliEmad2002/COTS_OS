/*
 * MotorEncoder.h
 *
 *  Created on: Aug 2, 2023
 *      Author: Ali Emad
 *
 * This driver handles magnetic encoder and optical encoder. Whether magnitude only
 * (single sensor), or magnitude and direction (dual sensor).
 *
 * This driver calculates accumulative position, and can calculate speed.
 */

#ifndef COTS_OS_INC_HAL_MOTORENCODER_MOTORENCODER_H_
#define COTS_OS_INC_HAL_MOTORENCODER_MOTORENCODER_H_

#include "FreeRTOS.h"

/*******************************************************************************
 * Structures:
 ******************************************************************************/
typedef struct{
	/*		PRIVATE		*/
	StackType_t puxTaskStack[configMINIMAL_STACK_SIZE];
	StaticTask_t xTaskStatic;
	TaskHandle_t xTask;

	/*		PUBLIC		*/
	uint8_t ucIsDualSensor; // expresses whether the handle is of dual sensor
							// (can measure magnitude and direction), or is a
							// single sensor (can measure magnitude only).
							// 0==> single,		1==> dual.
							// if single, "ucBPort" and "ucBPin" are ignored.

	uint8_t ucAPort;
	uint8_t ucAPin;

	uint8_t ucBPort;
	uint8_t ucBPin;

	int32_t iPos; // current position in (encoder ticks).
	int32_t iSpeed; // current speed in  (encoder ticks per second).

	uint32_t uiSpeedUpdatePeriodMs;
}xHOS_MotorEncoder_t;

/*******************************************************************************
 * API functions:
 ******************************************************************************/
/*
 * Initializes object and its task.
 *
 * Notes:
 * 		-	Must be called before scheduler start.
 *
 * 		-	All public parameters of the passed handle must be initialized first.
 *
 * 		-	"ucCaptureEdge" expresses which edge of sensor's output should the
 * 			driver increment pos counter on. 0==> falling, 1==> rising.
 *
 * 		-	"ucSpeedUpdatePri": 0==> soft real time, 1==> hard real time.
 * 			this is optional because it depends on user application.
 */
void vHOS_MotorEncoder_init(	xHOS_MotorEncoder_t* pxHandle,
								uint8_t ucCaptureEdge,
								uint8_t ucSpeedUpdatePri	);

/*
 * Enables object.
 *
 * Notes:
 * 		-	A "xHOS_MotorEncoder_t" is initially disabled.
 * 		-	This is an inline function.
 */
void vHOS_MotorEncoder_enable(xHOS_MotorEncoder_t* pxHandle);

/*
 * Disables object.
 *
 * Notes:
 * 		-	A "xHOS_MotorEncoder_t" is initially disabled.
 * 		-	This is an inline function.
 */
void vHOS_MotorEncoder_disable(xHOS_MotorEncoder_t* pxHandle);



#endif /* COTS_OS_INC_HAL_MOTORENCODER_MOTORENCODER_H_ */
