/*
 * HAL_OS.c
 *
 *  Created on: Jun 9, 2023
 *      Author: Ali Emad
 */

/*	LIB	*/
#include <stdint.h>

/*	FreeRTOS	*/
#include "FreeRTOS.h"
#include "task.h"

/*	MCAL (Ported)	*/
#include "MCAL_Port/Port_Clock.h"
#include "MCAL_Port/Port_DIO.h"
#include "MCAL_Port/Port_Print.h"
#include "MCAL_Port/Port_Breakpoint.h"
#include "MCAL_Port/Port_SPI.h"

/*	SELF	*/
#include "HAL/HAL_OS.h"

/*	Drivers' task initialization functions	*/
extern BaseType_t xSevenSegmentMux_initTask(void);
extern BaseType_t xHOS_SPI_init(void);

BaseType_t xHOS_init(void)
{
	BaseType_t xInitState;

#if configHOS_SEVEN_SEGMENT_EN
	xInitState = xSevenSegmentMux_initTask();
	if (xInitState == pdFAIL)
		return pdFAIL;
#endif	/*	configHOS_SEVEN_SEGMENT_EN	*/

#if configHOS_SPI_EN
	xInitState = xHOS_SPI_init();
	if (xInitState == pdFAIL)
		return pdFAIL;
#endif	/*	configHOS_SPI_EN	*/

	return pdPASS;
}




















