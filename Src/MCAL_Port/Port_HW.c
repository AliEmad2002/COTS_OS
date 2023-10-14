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

void vPort_HW_init(void)
{
	/*	Initialize peripherals' clocks	*/
	vPort_Clock_initPeriphClock();

	/*		UART1		*/
	vPort_UART_initHW(0);
	vPort_UART_setTransferDirection(0, 2);
	vPort_UART_setStopBitsLength(0, 1);
	vPort_UART_setBaudRate(0, 9600);
	vPort_AFIO_mapUart(0, 0);
	vPort_GPIO_initUartPins(0, 0, 0, 1);

	/*		UART2		*/
	vPort_UART_initHW(1);
	vPort_UART_setTransferDirection(1, 2);
	vPort_UART_setStopBitsLength(1, 1);
	vPort_UART_setBaudRate(1, 9600);
	vPort_AFIO_mapUart(1, 0);
	vPort_GPIO_initUartPins(1, 0, 0, 1);

	/*		UART3		*/
	vPort_UART_initHW(2);
	vPort_UART_setTransferDirection(2, 2);
	vPort_UART_setStopBitsLength(2, 1);
	vPort_UART_setBaudRate(2, 9600);
	vPort_AFIO_mapUart(2, 0);
	vPort_GPIO_initUartPins(2, 0, 0, 1);

	/*	PWR	*/
//	vPort_PWR_init();
//	vPort_PWR_setPvdThreshold(2900);
//	vPort_PWR_enablePvd();
//	vPort_PWR_setPvdCallback(vShutdownHandler, NULL);
//	VPORT_INTERRUPT_SET_PRIORITY(
//		xPortInterruptPwrPvdIrqNumber,
//		configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY	);
//	vPORT_INTERRUPT_ENABLE_IRQ(xPortInterruptPwrPvdIrqNumber);

//	/*	BKP	*/
//	vPort_BKP_init();
//
//	/*	RTC	*/
//	vPort_init();
//	vPort_RTC_enable();

	/*	USB	*/
	/*	PLEAE KEEP IN MIND: USB's SW is very much RAM, flash and execution time consuming	*/
//	vPort_USB_initHardware();
}
