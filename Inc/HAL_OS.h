/*
 * HAL_OS.h
 *
 *  Created on: Jun 8, 2023
 *      Author: Ali Emad
 */

#ifndef HAL_OS_INC_HAL_OS_H_
#define HAL_OS_INC_HAL_OS_H_

#include "FreeRTOS.h"
#include "Inc/Button/Button.h"
#include "Inc/SevenSegmentMux/SevenSegmentMux.h"
#include "Inc/SPI/SPI.h"
#include "Inc/TFT/TFT.h"

BaseType_t xHOS_init(void);



#endif /* HAL_OS_INC_HAL_OS_H_ */
