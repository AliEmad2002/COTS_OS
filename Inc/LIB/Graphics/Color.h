/*
 * Color.h
 *
 *  Created on: Aug 27, 2023
 *      Author: Ali Emad
 */

#ifndef COTS_OS_INC_LIB_GRAPHICS_COLOR_H_
#define COTS_OS_INC_LIB_GRAPHICS_COLOR_H_

#include "MCAL_Port/Port_ASM.h"


/*
 * "xLIB_Color16_t" handle should be written to using "vLIB_COLOR_SET16_FROM_565()".
 */
typedef uint16_t xLIB_Color16_t;

#define xLIB_COLOR_GET16_FROM_565(xR, xG, xB)	\
	(uiPort_ASM_reverse16((uint16_t)(xR | (xG << 5) | (xG << 11))))


#define xLIB_COLOR_WHITE	(xLIB_COLOR_GET16_FROM_565(31, 63, 31))

#define xLIB_COLOR_BLACK	(xLIB_COLOR_GET16_FROM_565(0, 0, 0))

#endif /* COTS_OS_INC_LIB_GRAPHICS_COLOR_H_ */
