/*
 * Port_USB.h
 *
 *  Created on: Sep 11, 2023
 *      Author: Ali Emad
 */

/*	Target checking	*/
#include "MCAL_Port/Port_Target.h"
#ifdef MCAL_PORT_TARGET_STM32F401RCT6


#ifndef COTS_OS_INC_MCAL_PORT_PORT_USB_H_
#define COTS_OS_INC_MCAL_PORT_PORT_USB_H_

//#define ucPORT_USB_ENABLE

#ifdef ucPORT_USB_ENABLE

/*
 * Please select device mode or host mode.
 */
/*
 * TODO: USB host mode is not yet completed.
 */
//#define ucPORT_USB_RUN_IN_HOST_MODE
//#define ucPORT_USB_RUN_IN_DEVICE_MODE

/*
 * Initializes USB hardware.
 */
void vPort_USB_initHardware(void);


#endif	/*	ucPORT_USB_ENABLE	*/

#endif /* COTS_OS_INC_MCAL_PORT_PORT_USB_H_ */


#endif /* Target checking */
