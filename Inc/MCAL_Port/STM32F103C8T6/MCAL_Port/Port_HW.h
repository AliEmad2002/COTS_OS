/*
 * Port_HW.h
 *
 *  Created on: Jun 17, 2023
 *      Author: Ali Emad
 */

/*	Target checking	*/
#include "MCAL_Port/Port_Target.h"
#ifdef MCAL_PORT_TARGET_STM32F103C8T6


#ifndef HAL_OS_PORT_PORT_HW_H_
#define HAL_OS_PORT_PORT_HW_H_

/*
 * This function is called on system startup. It initializes low level HW (MCAL).
 */
void vPort_HW_init(void);













#endif /* HAL_OS_PORT_PORT_HW_H_ */


#endif /* Target checking */