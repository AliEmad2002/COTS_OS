/*
 * Port_HW.c
 *
 *  Created on: Sep 25, 2023
 *      Author: Ali Emad
 */


#include "stdint.h"

#include "MCAL_Port/Port_HW.h"

#include "MCAL_Port/Port_Clock.h"
#include "MCAL_Port/Port_PWR.h"
#include "MCAL_Port/Port_BKP.h"
#include "MCAL_Port/Port_Interrupt.h"
#include "MCAL_Port/Port_RTC.h"
#include "MCAL_Port/Port_USB.h"
#include "MCAL_Port/Port_UART.h"
#include "MCAL_Port/Port_AFIO.h"
#include "MCAL_Port/Port_GPIO.h"
#include "MCAL_Port/Port_ADC.h"
#include "MCAL_Port/Port_I2C.h"
#include "MCAL_Port/Port_SPI.h"

void vPort_HW_init(void)
{
	/*	Initialize peripherals' clocks	*/
	vPort_Clock_initPeriphClock();

	/*	Initialize PWR	*/
	vPort_PWR_init();

	/*	Initialize BKP	*/
	vPort_BKP_init();

	/*	Initialize ADC	*/
	vPort_ADC_init(0);
	vPort_ADC_setChannelSampleTime(0, 5, 1);

	/*	Initialize USB	*/
	vPort_USB_initHardware();
}
