/*
 * HardwareDelayConfig.h
 *
 *  Created on: Jun 26, 2023
 *      Author: Ali Emad
 */

#ifndef HAL_OS_INC_HARDWAREDELAY_HARDWAREDELAYCONFIG_H_
#define HAL_OS_INC_HARDWAREDELAY_HARDWAREDELAYCONFIG_H_


/*	Maximum number of HW delay objects that can be created	*/
#define configHOS_HARDWARE_DELAY_MAX_NUMBER_OF_OBJECTS		1

/*	Numbers of HW timer units to be used	*/
static const uint8_t configHOS_HardwareDelayTimerUnits[configHOS_HARDWARE_DELAY_MAX_NUMBER_OF_OBJECTS] = {
	1
};


#endif /* HAL_OS_INC_HARDWAREDELAY_HARDWAREDELAYCONFIG_H_ */
