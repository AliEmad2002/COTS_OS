/*
 * HardwareDelay.h
 *
 *  Created on: Jun 26, 2023
 *      Author: Ali Emad
 */

#ifndef HAL_OS_INC_HARDWAREDELAY_HARDWAREDELAY_H_
#define HAL_OS_INC_HARDWAREDELAY_HARDWAREDELAY_H_

#include "FreeRTOS.h"
#include "semphr.h"
#include "HAL/HardwareDelay/HardwareDelayConfig.h"

#if configHOS_HARDWARE_DELAY_ENABLE

/*******************************************************************************
 * API structures:
 ******************************************************************************/
typedef struct{
	/*				Private				*/
	/*	These are internally used by the driver.	*/
	uint8_t ucTimerUnitNumber;
	uint32_t uiMaxCounterValue;
	StaticSemaphore_t xHWStaticMutex;
	SemaphoreHandle_t xHWMutex;

	/*				Public				*/
	/*	Must be taken first before using the "xHOS_HardwareDelay_t" object.	*/
	StaticSemaphore_t xStaticMutex;
	SemaphoreHandle_t xMutex;

	/*	Actual/accurate number of tick per second	*/
	uint32_t uiTicksPerSecond;
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
 * Initializes new object.
 * Returns pointer to the initialized object.
 *
 * Notes:
 * 		-	Number of previously created objects must be less than "configHOS_HARDWARE_DELAY_MAX_NUMBER_OF_OBJECTS"
 * 			by one, otherwise, there's no more HW resources to initialize and use.
 *
 * 		-	This driver uses HW timers from the port layer, so the timer units
 * 			defined in "configHOS_HardwareDelayTimerUnits[]" must not be used elsewhere.
 */
xHOS_HardwareDelay_t* pxHOS_HardwareDelay_initNewObject(xHOS_HardwareDelay_InitApproxFreq_t xFreqApproximate);

/*
 * Starts an RTOS blocking delay for a number of ticks. (i.e.: calling task would be blocked by FreeRTOS)
 *
 * Notes:
 * 		-	To guarantee correct timing, the calling task must be of a very high
 * 			priority. otherwise delay time may be larger than requested.
 */
void vHOS_HardwareDelay_delayTicks(xHOS_HardwareDelay_t* pxHandle, uint64_t uiTicks);

/*
 * Starts an RTOS blocking delay for a number of microseconds. (i.e.: calling task would be blocked by FreeRTOS)
 */
void vHOS_HardwareDelay_delayUs(xHOS_HardwareDelay_t* pxHandle, uint32_t uiUs);

#endif	/*	configHOS_HARDWARE_DELAY_ENABLE	*/

#endif /* HAL_OS_INC_HARDWAREDELAY_HARDWAREDELAY_H_ */
