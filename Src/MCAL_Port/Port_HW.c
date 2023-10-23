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

static void vInitUartUnit(uint8_t i)
{
	static const uint8_t pucMapArr[] = {1, 0, 0};
	vPort_UART_initHW(i);
	vPort_UART_setTransferDirection(i, 2);
	vPort_UART_setStopBitsLength(i, 1);
	vPort_UART_setBaudRate(i, 9600);
	vPort_AFIO_mapUart(i, pucMapArr[i]);
	vPort_GPIO_initUartPins(i, pucMapArr[i], 0, 1);

}

void vPort_HW_init(void)
{
	/*	Initialize peripherals' clocks	*/
	vPort_Clock_initPeriphClock();

	/*		UART		*/
	vInitUartUnit(0);
	vInitUartUnit(1);
	vInitUartUnit(2);

	/*	PWR	*/
	vPort_PWR_init();

	/*	BKP	*/
	vPort_BKP_init();

	/*	RTC	*/
	vPort_RTC_init();
	vPort_RTC_enable();

	/*	USB	*/
	/*	PLEAE KEEP IN MIND: USB's SW is very much RAM, flash and execution time consuming	*/
	vPort_USB_initHardware();
}
