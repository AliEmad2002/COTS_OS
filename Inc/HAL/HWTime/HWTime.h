/*
 * HWTime.h
 *
 *  Created on: Aug 28, 2023
 *      Author: Ali Emad
 *
 * As the RTOS tick is too slow and not acceptable for some use cases, this driver
 * uses a HW timer to provide much more accurate, non-blocing and thread-safe
 * timestamping and delay mechanisms.
 *
 * Notes:
 * 		-	This driver uses SW overflow counter, in a thread safe method (see
 * 			implementation in .c file for details), so there's absolutely no worries
 * 			about timestamp overflow.
 */

#ifndef COTS_OS_INC_HAL_HWTIME_HWTIME_H_
#define COTS_OS_INC_HAL_HWTIME_HWTIME_H_

#include "HAL/HWTime/HWTime_config.h"

typedef struct{
	/*		PRIVATE		*/

	/*	Time at which the delay ends	*/
	uint64_t ulReleaseTimestamp;

	/*	Binary semaphore that gets locked at start of the delay, unlocked at the end of it	*/
	SemaphoreHandle_t xSemaphore;
	StaticSemaphore_t xSemaphoreStatic;
}xHOS_HWTime_Delay_t;


/*
 * Initializes driver.
 *
 * Notes:
 * 		-	Must be called before scheduler start.
 */
void vHOS_HWTime_init(void);

/*
 * Gets timestamp.
 *
 * Notes:
 * 		-	This function is not ISR safe (Due to FreeRTOS considerations).
 * 			For ISR use, call "()" instead.
 */
uint64_t ulHOS_HWTime_getTimestamp(void);

/*
 * Gets timestamp from inside ISR.
 */
uint64_t ulHOS_HWTime_getTimestampFromISR(void);

/*
 * Converts time from ms to ticks.
 */
#define ulHOS_HWTime_MS_TO_TICKS(ulMs)	\
	(((uint64_t)(ulMs) * (uint64_t)uiHOS_HWTIME_TIMER_FREQ_ACTUAL) / 1000)

/*
 * Converts time from us to ticks.
 */
#define ulHOS_HWTime_US_TO_TICKS(uiUs)	\
	(((uiUs) * uiHOS_HWTIME_TIMER_FREQ_ACTUAL) / 1000000)

/*
 * Converts time from ticks to ms.
 */
#define ulHOS_HWTime_TICKS_TO_MS(ulTicks)	\
	(((uint64_t)(ulTicks) * (uint64_t)1000) / uiHOS_HWTIME_TIMER_FREQ_ACTUAL)

/*
 * Converts time from ticks to us.
 */
#define ulHOS_HWTime_TICKS_TO_US(ulTicks)	\
	(((uint64_t)(ulTicks) * (uint64_t)1000000) / uiHOS_HWTIME_TIMER_FREQ_ACTUAL)

/*
 * Initialize Delay handle.
 *
 * Notes:
 * 		-	This must be globally defined (In order for the HWTime code to be able
 * 			to deal with it when the task using it is not running).
 */
void vHOS_HWTime_initDelay(xHOS_HWTime_Delay_t* pxHandle);

/*
 * Configures delay handle.
 *
 * Notes:
 * 		-	The passed handle must not be currently in use (waiting for a previous
 * 			delay to end).
 */
void vHOS_HWTime_delay(xHOS_HWTime_Delay_t* pxHandle, uint32_t uiTicks);


































#endif /* COTS_OS_INC_HAL_HWTime_HWTime_H_ */
