/*
 * HardwareDelay.c
 *
 *  Created on: Jun 26, 2023
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
#include "HAL/HardwareDelay/HardwareDelay.h"


/*******************************************************************************
 * Overflow handlers:
 ******************************************************************************/
static void vCallback(void* pvParams)
{
	xHOS_HardwareDelay_t* pxHandle = (xHOS_HardwareDelay_t*)pvParams;

	/*	Stop counter	*/
	vPort_TIM_disableCounter(pxHandle->ucTimerUnitNumber);

	/*	Give HW mutex	*/
	BaseType_t xHighPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(pxHandle->xHWMutex, &xHighPriorityTaskWoken);
	portYIELD_FROM_ISR(xHighPriorityTaskWoken);
}

/*******************************************************************************
 * Helping (private) functions:
 ******************************************************************************/
static inline void vInitHWTimer(	xHOS_HardwareDelay_t* pxHandle,
									xHOS_HardwareDelay_InitApproxFreq_t xFreqApproximate	)
{
	uint8_t ucTimerUnitNumber = pxHandle->ucTimerUnitNumber;

	vPort_TIM_disableCounter(ucTimerUnitNumber);

	vPort_TIM_useInternalClockSource(ucTimerUnitNumber);

	switch(xFreqApproximate)
	{
	case xHOS_HardwareDelay_Frequency_10kHz:
		vPort_TIM_setPrescaler(ucTimerUnitNumber, uiPORT_TIM_PRESCALER_FOR_10_KHZ);
		break;

	case xHOS_HardwareDelay_Frequency_100kHz:
		vPort_TIM_setPrescaler(ucTimerUnitNumber, uiPORT_TIM_PRESCALER_FOR_100_KHZ);
		break;

	case xHOS_HardwareDelay_Frequency_500kHz:
		vPort_TIM_setPrescaler(ucTimerUnitNumber, uiPORT_TIM_PRESCALER_FOR_500_KHZ);
		break;
	}

	vPort_TIM_setModeNormal(ucTimerUnitNumber);

	vPort_TIM_clearOverflowFlag(ucTimerUnitNumber);

	vPort_TIM_setCallback(ucTimerUnitNumber, vCallback, (void*)pxHandle);

	vPort_TIM_enableOverflowInterrupt(ucTimerUnitNumber);

	vPort_TIM_setCountingDirUp(ucTimerUnitNumber);

	uint32_t uiMaxCounterVal = (1 << puiPortTimerCounterSizeInBits[ucTimerUnitNumber]) - 1;
	vPort_TIM_setCounterUpperLimit(ucTimerUnitNumber, uiMaxCounterVal);
}

static inline void vInitInterruptController(uint8_t ucTimerUnitNumber)
{
	vPort_Interrupt_setPriority(
		pxPortInterruptTimerOvfIrqNumberArr[ucTimerUnitNumber],
		configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);

	vPort_Interrupt_enableIRQ(pxPortInterruptTimerOvfIrqNumberArr[ucTimerUnitNumber]);
}

static inline void vInitHandleParams(	xHOS_HardwareDelay_t* pxHandle,
										uint8_t ucTimerUnitNumber,
										xHOS_HardwareDelay_InitApproxFreq_t xFreqApproximate	)
{
	pxHandle->ucTimerUnitNumber = ucTimerUnitNumber;

	pxHandle->uiMaxCounterValue = (1 << puiPortTimerCounterSizeInBits[ucTimerUnitNumber]) - 1;

	/*	create HW mutex (initially not available)	*/
	pxHandle->xHWMutex = xSemaphoreCreateBinaryStatic(&(pxHandle->xHWStaticMutex));
	configASSERT(pxHandle->xHWMutex != NULL);

	/*	create mutex (initially available)	*/
	pxHandle->xMutex = xSemaphoreCreateBinaryStatic(&(pxHandle->xStaticMutex));
	configASSERT(pxHandle->xMutex != NULL);
	xSemaphoreGive(pxHandle->xHWMutex);

	/*	get actual timer unit frequency	*/
	switch(xFreqApproximate)
	{
	case xHOS_HardwareDelay_Frequency_10kHz:
		pxHandle->uiTicksPerSecond = uiPORT_TIM_FREQ_ACTUAL_FOR_10_KHZ;
		break;

	case xHOS_HardwareDelay_Frequency_100kHz:
		pxHandle->uiTicksPerSecond = uiPORT_TIM_FREQ_ACTUAL_FOR_100_KHZ;
		break;

	case xHOS_HardwareDelay_Frequency_500kHz:
		pxHandle->uiTicksPerSecond = uiPORT_TIM_FREQ_ACTUAL_FOR_500_KHZ;
		break;
	}
}

static inline void vDelay(xHOS_HardwareDelay_t* pxHandle, uint32_t uiTicks)
{
	/*	Assure HW mutex is not available	*/
	xSemaphoreTake(pxHandle->xHWMutex, 0);

	/*	Start timer such that it overflows after "uiTicks"	*/
	uint32_t uiCounterVal = pxHandle->uiMaxCounterValue - uiTicks;
	vPort_TIM_writeCounter(pxHandle->ucTimerUnitNumber, uiCounterVal);
	vPort_TIM_enableCounter(pxHandle->ucTimerUnitNumber);

	/*	Take HW mutex (block until OVF_Hnadler gives it)	*/
	xSemaphoreTake(pxHandle->xHWMutex, portMAX_DELAY);
}

/*******************************************************************************
 * API functions:
 ******************************************************************************/
/*
 * See header file for info.
 */
void vHOS_HardwareDelay_init(
	xHOS_HardwareDelay_t* pxHandle,
	xHOS_HardwareDelay_InitApproxFreq_t xFreqApproximate	)
{
	/*	Init HW timer using port driver	*/
	vInitHWTimer(pxHandle, xFreqApproximate);

	/*	Init "xHOS_HardwareDelay_t" object parameters	*/
	vInitHandleParams(pxHandle, pxHandle->ucTimerUnitNumber, xFreqApproximate);

	/*	Init overflow interrupt in interrupt controller	*/
	vInitInterruptController(pxHandle->ucTimerUnitNumber);
}

/*
 * See header file for info.
 */
void vHOS_HardwareDelay_delayTicks(xHOS_HardwareDelay_t* pxHandle, uint64_t uiTicks)
{
	/*	get number of full counter overflows needed	*/
	uint32_t uiNumberOfRemainingFullOVFs = uiTicks / pxHandle->uiMaxCounterValue;

	/*	get number of remainder ticks	*/
	uint32_t uiNumberOfRemainderTicks = uiTicks % pxHandle->uiMaxCounterValue;

	/*	delay	*/
	while(uiNumberOfRemainingFullOVFs--)
	{
		vDelay(pxHandle, pxHandle->uiMaxCounterValue);
	}

	vDelay(pxHandle, uiNumberOfRemainderTicks);
}

/*
 * See header file for info.
 */
__attribute__((always_inline)) inline
void vHOS_HardwareDelay_delayUs(xHOS_HardwareDelay_t* pxHandle, uint32_t uiUs)
{
	uint64_t uiTicks = ((uint64_t)uiUs * (uint64_t)pxHandle->uiTicksPerSecond) / 1000000ul;
	vHOS_HardwareDelay_delayTicks(pxHandle, uiTicks);
}
























