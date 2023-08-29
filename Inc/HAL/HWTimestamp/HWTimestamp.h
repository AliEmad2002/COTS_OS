/*
 * HWTimestamp.h
 *
 *  Created on: Aug 28, 2023
 *      Author: Ali Emad
 *
 * As the RTOS tick is too slow and not acceptable for some use cases, this driver
 * uses a HW timer is used to provide much more accurate and thread-safe timestamping
 * mechanism.
 *
 * Notes:
 * 		-	This driver does not need more than one handle, so it has no structures
 * 			and is statically given a certain time in config.h file.
 *
 * 		-	This driver uses SW overflow counter, in a thread safe method (see
 * 			implementation in .c file for details), so there's absolutely no worries
 * 			about timestamp overflow.
 */

#ifndef COTS_OS_INC_HAL_HWTIMESTAMP_HWTIMESTAMP_H_
#define COTS_OS_INC_HAL_HWTIMESTAMP_HWTIMESTAMP_H_

#include "HAL/HWTimestamp/HWTimestamp_config.h"

/*
 * Initializes driver.
 *
 * Notes:
 * 		-	Must be called before scheduler start.
 */
void vHOS_HWTimestamp_init(void);

/*
 * Gets timestamp.
 *
 * Notes:
 * 		-	This function is not ISR safe (Due to FreeRTOS considerations).
 * 			For ISR use, call "()" instead.
 */
uint64_t ulHOS_HWTimestamp_getTimestamp(void);

/*
 * Gets timestamp from inside ISR.
 */
uint64_t ulHOS_HWTimestamp_getTimestampFromISR(void);

/*
 * Converts time from ms to ticks.
 */
#define ulHOS_HWTIMESTAMP_MS_TO_TICKS(uiMs)	\
	(((uiMs) * uiHOS_HWTIMESTAMP_TIMER_FREQ_ACTUAL) / 1000)

/*
 * Converts time from us to ticks.
 */
#define ulHOS_HWTIMESTAMP_US_TO_TICKS(uiUs)	\
	(((uiUs) * uiHOS_HWTIMESTAMP_TIMER_FREQ_ACTUAL) / 1000000)

#endif /* COTS_OS_INC_HAL_HWTIMESTAMP_HWTIMESTAMP_H_ */
