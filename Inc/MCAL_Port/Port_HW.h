/*
 * Port_HW.h
 *
 *  Created on: Jun 17, 2023
 *      Author: Ali Emad
 */

#ifndef HAL_OS_PORT_PORT_HW_H_
#define HAL_OS_PORT_PORT_HW_H_

#include "MCAL_Port/Port_PWR.h"
#include "MCAL_Port/Port_BKP.h"
#include "MCAL_Port/Port_Interrupt.h"
#include "MCAL_Port/Port_RTC.h"
#include "MCAL_Port/Port_USB.h"

extern void vShutdownHandler(void* pvParams);

/*
 * This function is called on system startup. It initializes low level HW (MCAL).
 */
static inline void vPort_HW_init(void)
{
	/*		UART1		*/
//	vPort_UART_initHW(0);
//	vPort_UART_setTransferDirection(0, 2);
//	vPort_UART_setStopBitsLength(0, 1);
//	vPort_UART_setBaudRate(0, 9600);
//	vPort_AFIO_mapUart(0, 0);
//	vPort_GPIO_initUartPins(0, 0, 0, 1);

	/*	PWR	*/
//	vPort_PWR_init();
//	vPort_PWR_setPvdThreshold(2900);
//	vPort_PWR_enablePvd();
//	vPort_PWR_setPvdCallback(vShutdownHandler, NULL);
//	vPort_Interrupt_setPriority(
//		xPortInterruptPwrPvdIrqNumber,
//		configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY	);
//	vPort_Interrupt_enableIRQ(xPortInterruptPwrPvdIrqNumber);

//	/*	BKP	*/
//	vPort_BKP_init();
//
//	/*	RTC	*/
//	vPort_init();
//	vPort_RTC_enable();

	/*	USB	*/
	vPort_USB_initHardware();
}













#endif /* HAL_OS_PORT_PORT_HW_H_ */
