/*
 * Port_HW.h
 *
 *  Created on: Jun 17, 2023
 *      Author: Ali Emad
 */

#ifndef HAL_OS_PORT_PORT_HW_H_
#define HAL_OS_PORT_PORT_HW_H_

#include "MCAL_Port/Port_AFIO.h"
#include "MCAL_Port/Port_GPIO.h"
#include "MCAL_Port/Port_UART.h"


/*
 * This function is called on system startup. It initializes low level HW (MCAL).
 */
static inline void vPort_HW_init(void)
{
	vPort_UART_initHW(0);
	vPort_UART_setTransferDirection(0, 2);
	vPort_UART_setStopBitsLength(0, 1);
	vPort_UART_setBaudRate(0, 9600);

	vPort_AFIO_mapUart(0, 0);
	vPort_GPIO_initUartPins(0, 0, 1, 1);
}













#endif /* HAL_OS_PORT_PORT_HW_H_ */
