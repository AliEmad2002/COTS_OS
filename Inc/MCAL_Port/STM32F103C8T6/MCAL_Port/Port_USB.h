/*
 * Port_USB.h
 *
 *  Created on: Sep 11, 2023
 *      Author: Ali Emad
 */

/*	Target checking	*/
#include "MCAL_Port/Port_Target.h"
#ifdef MCAL_PORT_TARGET_STM32F103C8T6


#ifndef COTS_OS_INC_MCAL_PORT_PORT_USB_H_
#define COTS_OS_INC_MCAL_PORT_PORT_USB_H_


/*
 * Initializes USB hardware.
 */
void vPort_USB_initHardware(void);




#endif /* COTS_OS_INC_MCAL_PORT_PORT_USB_H_ */


#endif /* Target checking */