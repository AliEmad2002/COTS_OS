/*
 * HWTimestamp.c
 *
 *  Created on: Aug 28, 2023
 *      Author: Ali Emad
 */

/*	LIB	*/
#include <stdint.h>

/*	FreeRTOS	*/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/*	MCAL (Ported)	*/
#include "MCAL_Port/Port_Timer.h"
#include "MCAL_Port/Port_Interrupt.h"

/*	SELF	*/
#include "HAL/HWTimestamp/HWTimestamp.h"

/*******************************************************************************
 * Static / Global variables:
 ******************************************************************************/
static uint32_t uiOvfCount = 0;

static uint32_t uiMaxCounterVal;

/*******************************************************************************
 * Overflow handler:
 ******************************************************************************/
static void vCallback(void* pvParams)
{
	/*	Increment overflow counter	*/
	uiOvfCount++;
}


/*******************************************************************************
 * Helping functions / macros:
 ******************************************************************************/
static inline void vInitHWTimer(void)
{
	uint8_t ucTimerUnitNumber = ucHOS_HWTIMESTAMP_TIMER_UNIT_NUMBER;

	uiMaxCounterVal =
		(1 << puiPortTimerCounterSizeInBits[ucHOS_HWTIMESTAMP_TIMER_UNIT_NUMBER]) - 1;

	vPort_TIM_disableCounter(ucTimerUnitNumber);

	vPort_TIM_useInternalClockSource(ucTimerUnitNumber);

	vPort_TIM_setPrescaler(ucTimerUnitNumber, uiHOS_HWTIMESTAMP_TIMER_PRESCALER);

	vPort_TIM_setModeNormal(ucTimerUnitNumber);

	vPort_TIM_clearOverflowFlag(ucTimerUnitNumber);

	vPort_TIM_setCallback(ucTimerUnitNumber, vCallback, NULL);

	vPort_TIM_enableOverflowInterrupt(ucTimerUnitNumber);

	vPort_TIM_setCountingDirUp(ucTimerUnitNumber);

	vPort_TIM_setCounterUpperLimit(ucTimerUnitNumber, uiMaxCounterVal);

	vPort_TIM_enableCounter(ucTimerUnitNumber);
}

static inline void vInitInterruptController()
{
	uint8_t ucTimerUnitNumber = ucHOS_HWTIMESTAMP_TIMER_UNIT_NUMBER;

	vPort_Interrupt_setPriority(
		pxPortInterruptTimerOvfIrqNumberArr[ucTimerUnitNumber],
		configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);

	vPort_Interrupt_enableIRQ(pxPortInterruptTimerOvfIrqNumberArr[ucTimerUnitNumber]);
}



/*
 * See header for info.
 */
void vHOS_HWTimestamp_init(void)
{
	/*	Initialize HW timer	*/
	vInitHWTimer();

	/*	Initialize interrupt	*/
	vInitInterruptController();
}

/*
 * See header for info.
 */
uint64_t ulHOS_HWTimestamp_getTimestamp(void)
{
	//return uiOvfCount * uiMaxCounterVal + uiPORT_TIM_READ_COUNTER(ucHOS_HWTIMESTAMP_TIMER_UNIT_NUMBER);

	uint64_t ulTimestamp;

	taskENTER_CRITICAL();
	{
		/*	1. Read 1st counter value	*/
		register uint32_t uiCNT0 = uiPORT_TIM_READ_COUNTER(ucHOS_HWTIMESTAMP_TIMER_UNIT_NUMBER);

		/*	2. Wait for one tick to pass	*/
		while(uiPORT_TIM_READ_COUNTER(ucHOS_HWTIMESTAMP_TIMER_UNIT_NUMBER) != uiCNT0);

		/*	3. Read OVF flag	*/
		register uint32_t uiOvfFlag = ucPORT_TIM_GET_OVF_FLAG(ucHOS_HWTIMESTAMP_TIMER_UNIT_NUMBER);

		/*	4. Read 2nd counter value	*/
		register uint32_t uiCNT1 = uiPORT_TIM_READ_COUNTER(ucHOS_HWTIMESTAMP_TIMER_UNIT_NUMBER);

		/*
		 * 5. 	If overflow occurred, find out whether it occurred after reading
		 * 		the 1st counter value, or before it
		 */
		register uint32_t uiOvfOccuredBeforeCnt0 = (uiCNT0 < uiCNT1) && uiOvfFlag;

		/*
		 * 6.	Has overflow occurred before reading 1st counter value, and as
		 * 		the flag was not cleared until it was read in this critical section,
		 * 		this means that overflow occurred exactly after entering the critical
		 * 		section. Thus, "uiOvfCount" won't be incremented. Therefore, this
		 * 		function adds 1 to it when calculating the timestamp, to assure
		 * 		timestamp is valid.
		 */
		if (uiOvfOccuredBeforeCnt0)
			ulTimestamp = (uiOvfCount + 1) * uiMaxCounterVal + uiCNT0;

		/*
		 * 7.	Otherwise, has overflow occurred after reading 1st counter value,
		 * 		there's no need for adding 1 to the "uiOvfCount" in this function,
		 * 		as "uiCNT0" would be nearly equal to "uiMaxCounterVal", and
		 * 		"uiOvfCount" would be incremented safely in the ISR after exiting
		 * 		this critical section.
		 *
		 * 		Also, has no overflow occurred at all, no need for adding 1 to
		 * 		the "uiOvfCount".
		 */
		else
			ulTimestamp = uiOvfCount * uiMaxCounterVal + uiCNT0;
	}
	taskEXIT_CRITICAL();

	return ulTimestamp;
}

/*
 * See header for info.
 */
uint64_t ulHOS_HWTimestamp_getTimestampFromISR(void)
{
	uint64_t ulTimestamp;

	/*
	 * Same implementation as the previous function, only difference is that this
	 * function uses the ISR-safe version of entering and exiting critical section.
	 */
	UBaseType_t uxSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR();
	{
		register uint32_t uiCNT0 = uiPORT_TIM_READ_COUNTER(ucHOS_HWTIMESTAMP_TIMER_UNIT_NUMBER);

		while(uiPORT_TIM_READ_COUNTER(ucHOS_HWTIMESTAMP_TIMER_UNIT_NUMBER) != uiCNT0);

		register uint32_t uiOvfFlag = ucPORT_TIM_GET_OVF_FLAG(ucHOS_HWTIMESTAMP_TIMER_UNIT_NUMBER);

		register uint32_t uiCNT1 = uiPORT_TIM_READ_COUNTER(ucHOS_HWTIMESTAMP_TIMER_UNIT_NUMBER);

		register uint32_t uiOvfOccuredBeforeCnt0 = (uiCNT0 < uiCNT1) && uiOvfFlag;

		if (uiOvfOccuredBeforeCnt0)
			ulTimestamp = (uiOvfCount + 1) * uiMaxCounterVal + uiCNT0;
		else
			ulTimestamp = uiOvfCount * uiMaxCounterVal + uiCNT0;
	}
	taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus);

	return ulTimestamp;

}
