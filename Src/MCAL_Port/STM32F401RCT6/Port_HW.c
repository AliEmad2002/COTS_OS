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
#include "MCAL_Port/Port_UART.h"
#include "MCAL_Port/Port_ADC.h"
#include "MCAL_Port/Port_RTC.h"
#include "MCAL_Port/Port_USB.h"
#include "MCAL_Port/Port_I2C.h"


void vPort_HW_init(void)
{
	/*	Initialize peripherals' clocks	*/
	vPort_Clock_initPeriphClock();

	/*	Initialize RTC	*/
	vPort_RTC_init();
	vPort_RTC_enable();

	/*	Initialize ADC	*/
	vPort_ADC_init(0);

	/*	Initialize UART	*/
	vPort_UART_initHW(0);
	vPort_UART_setTransferDirection(0, 2);
	vPort_UART_setStopBitsLength(0, 1);
	vPort_UART_setBaudRate(0, 9600);
	vPort_GPIO_initUartPins(0, 1, 1);

	/*	Initialize I2C	*/
	xPort_I2C_HW_Conf_t xI2cConf = {
		.ucClockMode = 0,
		.ucIsAddress7Bit = 1,
		.uiSclFrequencyHz = 100000,
		.uiMaxRisingTimeNs = 1000
	};
	vPort_I2C_initHardware(0, &xI2cConf);

	/*	Initialize USB	*/
	vPort_USB_initHardware();
}


#endif /* Target checking */