/*
 * Port_HW.c
 *
 *  Created on: Sep 25, 2023
 *      Author: Ali Emad
 */

/*	Target checking	*/
#include "MCAL_Port/Port_Target.h"
#ifdef MCAL_PORT_TARGET_STM32F401RCT6



#include "stdint.h"

#include "MCAL_Port/Port_HW.h"

#include "MCAL_Port/Port_Clock.h"
#include "MCAL_Port/Port_Interrupt.h"
#include "MCAL_Port/Port_GPIO.h"
#include "MCAL_Port/Port_SPI.h"
#include "MCAL_Port/Port_Timer.h"
#include "MCAL_Port/Port_DIO.h"
#include "MCAL_Port/Port_DAC.h"


void vPort_HW_init(void)
{
	/*	Initialize peripherals' clocks	*/
	vPort_Clock_initPeriphClock();

	/*	Initialize RTC	*/
	vPort_RTC_init();
	vPort_RTC_enable();

	/*	Initialize ADC	*/
	vPort_ADC_init(0);

	/*	Initialize USB	*/
	vPort_USB_initHardware();
}


#endif /* Target checking */
