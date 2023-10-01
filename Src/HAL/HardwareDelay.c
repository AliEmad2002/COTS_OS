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
	vPORT_TIM_DISABLE_COUNTER(pxHandle->ucTimerUnitNumber);

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

	vPORT_TIM_DISABLE_COUNTER(ucTimerUnitNumber);

	vPORT_TIM_USE_INTERNAL_CLOCK_SOURCE(ucTimerUnitNumber);

	switch(xFreqApproximate)
	{
	case xHOS_HardwareDelay_Frequency_10kHz:
		VPORT_TIM_SET_PRESCALER(ucTimerUnitNumber, uiPORT_TIM_PRESCALER_FOR_10_KHZ);
		break;

	case xHOS_HardwareDelay_Frequency_100kHz:
		VPORT_TIM_SET_PRESCALER(ucTimerUnitNumber, uiPORT_TIM_PRESCALER_FOR_100_KHZ);
		break;

	case xHOS_HardwareDelay_Frequency_500kHz:
		VPORT_TIM_SET_PRESCALER(ucTimerUnitNumber, uiPORT_TIM_PRESCALER_FOR_500_KHZ);
		break;
	}

	vPORT_TIM_CLEAR_OVF_FLAG(ucTimerUnitNumber);

	vPort_TIM_setOvfCallback(ucTimerUnitNumber, vCallback, (void*)pxHandle);

	vPORT_TIM_ENABLE_OVF_INTERRUPT(ucTimerUnitNumber);

	vPORT_TIM_SET_COUNTING_DIR_UP(ucTimerUnitNumber);

	uint32_t uiMaxCounterVal = (1ul << pucPortTimerCounterSizeInBits[ucTimerUnitNumber]) - 1;
	vPORT_TIM_SET_COUNTER_UPPER_LIMIT(ucTimerUnitNumber, uiMaxCounterVal);
}

static inline void vInitInterruptController(uint8_t ucTimerUnitNumber)
{
	VPORT_INTERRUPT_SET_PRIORITY(
		pxPortInterruptTimerOvfIrqNumberArr[ucTimerUnitNumber],
		configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1);

	vPORT_INTERRUPT_ENABLE_IRQ(pxPortInterruptTimerOvfIrqNumberArr[ucTimerUnitNumber]);
}

static inline void vInitHandleParams(	xHOS_HardwareDelay_t* pxHandle,
										uint8_t ucTimerUnitNumber,
										xHOS_HardwareDelay_InitApproxFreq_t xFreqApproximate	)
{
	pxHandle->ucTimerUnitNumber = ucTimerUnitNumber;

	pxHandle->uiMaxCounterValue = (1ul << pucPortTimerCounterSizeInBits[ucTimerUnitNumber]) - 1;

	/*	create HW mutex (initially not available)	*/
	pxHandle->xHWMutex = xSemaphoreCreateBinaryStatic(&(pxHandle->xHWStaticMutex));
	configASSERT(pxHandle->xHWMutex != NULL);

	/*	create mutex (initially available)	*/
	pxHandle->xMutex = xSemaphoreCreateBinaryStatic(&(pxHandle->xStaticMutex));
	configASSERT(pxHandle->xMutex != NULL);
	xSemaphoreGive(pxHandle->xMutex);

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
	vPORT_TIM_WRITE_COUNTER(pxHandle->ucTimerUnitNumber, uiCounterVal);
	vPORT_TIM_ENABLE_COUNTER(pxHandle->ucTimerUnitNumber);

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
























