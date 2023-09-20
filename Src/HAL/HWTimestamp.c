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
	register uint32_t uiCNT;
	register uint32_t uiOvfFlag;
	register uint32_t uiOvfOccuredBeforeCnt;

	taskENTER_CRITICAL();
	{
		/*	Read counter value and OVF flag	*/
		uiCNT = uiPORT_TIM_READ_COUNTER(ucHOS_HWTIMESTAMP_TIMER_UNIT_NUMBER);
		uiOvfFlag = ucPORT_TIM_GET_OVF_FLAG(ucHOS_HWTIMESTAMP_TIMER_UNIT_NUMBER);

		/*
		 * If overflow flag was set, then one of two possibilities has happened:
		 * 		-	Flag was set after reading the counter register value.
		 * 		-	Flag was set before reading the counter register value.
		 *
		 * This could be detected from the value of "uiCNT". Had it been less than
		 * half the top counter value, then the overflow has happened before reading.
		 * Otherwise, it would have happened after reading.
		 */
		uiOvfOccuredBeforeCnt = (uiCNT < uiMaxCounterVal/2) && uiOvfFlag;

		/*
		 * Has overflow occurred before reading counter value, and as the flag was not
		 * cleared until it was read in this critical section, this means that
		 * overflow occurred exactly after entering the critical section. Thus,
		 * "uiOvfCount" won't be incremented. Therefore, this function adds 1 to it
		 * when calculating the timestamp, to assure timestamp is valid.
		 */
		if (uiOvfOccuredBeforeCnt)
			ulTimestamp = (uiOvfCount + 1) * uiMaxCounterVal + uiCNT;

		/*
		 * Otherwise, has overflow occurred after reading counter value, there's
		 * no need for adding 1 to the "uiOvfCount" in this function, as "uiCN0"
		 * would be nearly equal to "uiMaxCounterVal", and "uiOvfCount" would be
		 * incremented safely in the ISR after exiting this critical section.
		 *
		 * Also, has no overflow occurred at all, no need for adding 1 to the
		 * "uiOvfCount".
		 */
		else
			ulTimestamp = uiOvfCount * uiMaxCounterVal + uiCNT;
	}
	taskEXIT_CRITICAL();



	return ulTimestamp;
}

/*
 * See header for info.
 */
uint64_t ulHOS_HWTimestamp_getTimestampFromISR(void)
{
	/*
	 * Same implementation as the previous function, only difference is that this
	 * function uses the ISR-safe version of entering and exiting critical section.
	 */
	//return uiOvfCount * uiMaxCounterVal + uiPORT_TIM_READ_COUNTER(ucHOS_HWTIMESTAMP_TIMER_UNIT_NUMBER);

	uint64_t ulTimestamp;
	register uint32_t uiCNT;
	register uint32_t uiOvfFlag;
	register uint32_t uiOvfOccuredBeforeCnt;

	UBaseType_t uxSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR();
	{
		uiCNT = uiPORT_TIM_READ_COUNTER(ucHOS_HWTIMESTAMP_TIMER_UNIT_NUMBER);
		uiOvfFlag = ucPORT_TIM_GET_OVF_FLAG(ucHOS_HWTIMESTAMP_TIMER_UNIT_NUMBER);
		uiOvfOccuredBeforeCnt = (uiCNT < uiMaxCounterVal/2) && uiOvfFlag;

		if (uiOvfOccuredBeforeCnt)
			ulTimestamp = (uiOvfCount + 1) * uiMaxCounterVal + uiCNT;
		else
			ulTimestamp = uiOvfCount * uiMaxCounterVal + uiCNT;
	}
	taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus);



	return ulTimestamp;

}
