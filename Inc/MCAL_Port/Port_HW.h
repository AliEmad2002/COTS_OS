/*
 * Port_HW.h
 *
 *  Created on: Jun 17, 2023
 *      Author: Ali Emad
 */

#ifndef HAL_OS_PORT_PORT_HW_H_
#define HAL_OS_PORT_PORT_HW_H_

#include "MCAL_Port/Port_USB.h"


/*
 * This function is called on system startup. It initializes low level HW (MCAL).
 */
static inline void vPort_HW_init(void)
{
	/*	USB	*/
	vPort_USB_initHardware();
}













#endif /* HAL_OS_PORT_PORT_HW_H_ */
