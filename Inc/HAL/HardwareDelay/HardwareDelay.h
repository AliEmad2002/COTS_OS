/*
 * HardwareDelay.h
 *
 *  Created on: Jun 26, 2023
 *      Author: Ali Emad
 *
 * Notes:
 * 		-	When tested on STM32F103C8T6 @72MHz, minimum possible delay was about
 * 			40us.
 *
 * 		-	For delays less than this (probably for signal generation), PWM mode
 * 			and OPM (one-pulse mode) could be used achieve signal's timing.
 */

#ifndef HAL_OS_INC_HARDWAREDELAY_HARDWAREDELAY_H_
#define HAL_OS_INC_HARDWAREDELAY_HARDWAREDELAY_H_

#include "FreeRTOS.h"
#include "semphr.h"


/*******************************************************************************
 * API structures:
 ******************************************************************************/
typedef struct{
	/*				Private				*/
	/*	These are internally used by the driver.	*/
	uint32_t uiMaxCounterValue;
	StaticSemaphore_t xHWStaticMutex;
	SemaphoreHandle_t xHWMutex;

	/*	Must be taken first before using the "xHOS_HardwareDelay_t" object.	*/
	StaticSemaphore_t xStaticMutex;
	SemaphoreHandle_t xMutex;

	/*				Public				*/
	/*	Actual/accurate number of tick per second (Read only)	*/
	uint32_t uiTicksPerSecond;

	uint8_t ucTimerUnitNumber;
}xHOS_HardwareDelay_t;

/*
 * HardwareDelay object initialization frequency (ticks per second).
 *
 * Notes:
 * 		-	These are approximation values for initialization, after initialization,
 * 			real frequency may be a little different based on the HW properties.
 * 			real frequency could be then found at "uiTicksPerSecond" in the "xHOS_HardwareDelay_t"
 * 			object.
 *
 * 		-	Since 1kHz and below can be achieved using FreeRTOS's SW timer, this
 * 			HardwareDelay driver would focus on higher frequencies.
 */
typedef enum{
	xHOS_HardwareDelay_Frequency_10kHz,
	xHOS_HardwareDelay_Frequency_100kHz,
	xHOS_HardwareDelay_Frequency_500kHz
}xHOS_HardwareDelay_InitApproxFreq_t;

/*
 * Initializes handle.
 */
void vHOS_HardwareDelay_init(
	xHOS_HardwareDelay_t* pxHandle,
	xHOS_HardwareDelay_InitApproxFreq_t xFreqApproximate	);

/*	Locks handle	*/
uint8_t ucHOS_HardwareDelay_lock(
	xHOS_HardwareDelay_t* pxHandle,
	TickType_t xTimeout	);

/*	Unlocks handle	*/
void vHOS_HardwareDelay_unlock(xHOS_HardwareDelay_t* pxHandle);

/*
 * Starts an RTOS blocking delay for a number of ticks. (i.e.: calling task
 * would be blocked by FreeRTOS)
 *
 * Notes:
 * 		-	To guarantee correct timing, the calling task must be of a very high
 * 			priority. otherwise delay time may be larger than requested.
 */
void vHOS_HardwareDelay_delayTicks(xHOS_HardwareDelay_t* pxHandle, uint64_t uiTicks);

/*
 * Starts an RTOS blocking delay for a number of microseconds. (i.e.: calling
 * task would be blocked by FreeRTOS)
 *
 * Notes:
 * 		-	To guarantee correct timing, the calling task must be of a very high
 * 			priority. otherwise delay time may be larger than requested.
 */
void vHOS_HardwareDelay_delayUs(xHOS_HardwareDelay_t* pxHandle, uint32_t uiUs);












#endif /* HAL_OS_INC_HARDWAREDELAY_HARDWAREDELAY_H_ */
