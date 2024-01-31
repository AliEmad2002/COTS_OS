/*
 * Port_RNG.h
 *
 *  Created on: Sep 7, 2023
 *      Author: Ali Emad
 *
 * This driver implements random number generator.
 */

/*	Target checking	*/
#include "MCAL_Port/Port_Target.h"
#ifdef MCAL_PORT_TARGET_STM32F103C8T6


#ifndef INC_MCAL_PORT_PORT_RNG_H_
#define INC_MCAL_PORT_PORT_RNG_H_

#include <stdlib.h>

/*
 * If the used target has RNG peripheral, this function initializes it.
 * Otherwise, this function initializes the C-standard library random number generator.
 *
 * In case of using the C-standard library RNG, it is better to initialize it with
 * a value derived from a random source, such as a floating ADC channel, current
 * RTC timestamp (not so random though), etc...
 */
void vPort_RNG_init(void);

#define iPORT_RNG_GET_RAND()	(	rand()	)


#endif /* INC_MCAL_PORT_PORT_RNG_H_ */


#endif /* Target checking */