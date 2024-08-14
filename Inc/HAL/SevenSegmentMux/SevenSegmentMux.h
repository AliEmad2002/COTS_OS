/*
 * SevenSegmentMux.h
 *
 *  Created on: Jun 9, 2023
 *      Author: Ali Emad
 */

#ifndef HAL_OS_INC_SEVENSEGMENTMUX_SEVENSEGMENTMUX_H_
#define HAL_OS_INC_SEVENSEGMENTMUX_SEVENSEGMENTMUX_H_

#include "HAL/SevenSegmentMux/SevenSegmentMuxConfig.h"
#include "MCAL_Port/Port_DIO.h"

#define uiHOS_SEVENSEGMENTMUX_STACK_SZ	(200)

/*
 * "xHOS_SevenSegmentMux_t" structure.
 */
typedef struct{
	/*		PRIVATE		*/
	StackType_t puxTaskStack[uiHOS_SEVENSEGMENTMUX_STACK_SZ];
	StaticTask_t xTaskStatic;
	TaskHandle_t xTask;

	uint8_t ucCurrentActiveDigit;

	/*		PUBLIC		*/
	/*	Configuration parameters	*/
	uint8_t* pxSegmentPortNumberArr;
	uint8_t* pxSegmentPinNumberArr;

	uint8_t* pxDigitEnablePortNumberArr;
	uint8_t* pxDigitEnablePinNumberArr;

	uint8_t ucSegmentActiveLevel : 1;

	uint8_t ucEnableActiveLevel : 1;

	uint8_t ucIsEnabled : 1;

	uint8_t ucNumberOfDigits;

	/*	Runtime changing parameters	*/
	uint8_t* pucDisplayBuffer;

	int8_t ucPointIndex;

	uint32_t uiUpdatePeriodMs; // recommended: 10ms
}xHOS_SevenSegmentMux_t;

/*
 * Initializes "xHOS_SevenSegmentMux_t" object.
 *
 * Notes:
 * 		-	Must be called before scheduler start.
 *
 * 		-	All configuration parameters, and "uiUpdatePeriodMs" must be
 * 			initialized first.
 *
 * 		-	Runtime parameters are initialized by this function as follows:
 * 				-	"pucDisplayBuffer" elements are all set to zero.
 * 				-	"cPointIndex" is set to -1.
 */
void vHOS_SevenSegmentMux_init(xHOS_SevenSegmentMux_t* pxHandle);

/*
 * Changes current display value.
 */
void vHOS_SevenSegmentMux_write(	xHOS_SevenSegmentMux_t* pxHandle,
									uint32_t uiNum,
									uint8_t ucPointIndex	);

/*
 * Enables "xHOS_SevenSegmentMux_t" object.
 *
 * Notes:
 * 		-	Objects are initially enabled on creation.
 * 		-	This function is inline.
 */
void vHOS_SevenSegmentMux_Enable(xHOS_SevenSegmentMux_t* pxHandle);

/*
 * Disables "xHOS_SevenSegmentMux_t" object.
 *
 * Notes:
 * 		-	Objects are initially enabled on creation.
 */
void vHOS_SevenSegmentMux_Disable(xHOS_SevenSegmentMux_t* pxHandle);

#endif /* HAL_OS_INC_SEVENSEGMENTMUX_SEVENSEGMENTMUX_H_ */
