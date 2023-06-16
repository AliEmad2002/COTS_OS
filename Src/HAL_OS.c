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
#include "Port/Port_Clock.h"
#include "Port/Port_DIO.h"
#include "Port/Port_Print.h"
#include "Port/Port_Breakpoint.h"
#include "Port/Port_SPI.h"

/*	SELF	*/
#include "Inc/HAL_OS.h"

/*	Drivers' task initialization functions	*/
extern BaseType_t xButton_initTask(void);
extern BaseType_t xSevenSegmentMux_initTask(void);
extern BaseType_t xHOS_SPI_init(void);
extern void vHOS_SPI_initAllUnitsHardware(void);

BaseType_t xHOS_init(void)
{
	BaseType_t xInitState;

	vPort_Clock_init();

#if configHOS_BUTTON_EN
	xInitState = xButton_initTask();
	if (xInitState == pdFAIL)
		return pdFAIL;
#endif	/*	configHOS_BUTTON_EN	*/

#if configHOS_SEVEN_SEGMENT_EN
	xInitState = xSevenSegmentMux_initTask();
	if (xInitState == pdFAIL)
		return pdFAIL;
#endif	/*	configHOS_SEVEN_SEGMENT_EN	*/

#if configHOS_SPI_EN
	vHOS_SPI_initAllUnitsHardware();
	xInitState = xHOS_SPI_init();
	if (xInitState == pdFAIL)
		return pdFAIL;
#endif	/*	configHOS_SPI_EN	*/

	return pdPASS;
}




















