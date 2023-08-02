/*
 * HAL_OS.h
 *
 *  Created on: Jun 8, 2023
 *      Author: Ali Emad
 */

#ifndef HAL_OS_INC_HAL_OS_H_
#define HAL_OS_INC_HAL_OS_H_

#include "FreeRTOS.h"
#include "RTOS_PRI_Config.h"
#include "HAL/Button/Button.h"
#include "HAL/SevenSegmentMux/SevenSegmentMux.h"
#include "HAL/SPI/SPI.h"
#include "HAL/I2C/I2C.h"
#include "HAL/TFT/TFT.h"
#include "HAL/HardwareDelay/HardwareDelay.h"
#include "HAL/RotaryEncoder/RotaryEncoder.h"
#include "HAL/MPU6050/MPU6050.h"
#include "HAL/RF/RF.h"
#include "HAL/MotorEncoder/MotorEncoder.h"
#include "HAL/Stepper/Stepper.h"
#include "HAL/Stepper/StepperSynchronizer.h"

BaseType_t xHOS_init(void);



#endif /* HAL_OS_INC_HAL_OS_H_ */
