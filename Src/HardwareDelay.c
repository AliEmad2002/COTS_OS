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
#include "Port/Port_Timer.h"
#include "Port/Port_Interrupt.h"

/*	SELF	*/
#include "Inc/HardwareDelay/HardwareDelay.h"

/*******************************************************************************
 * Static objects:
 ******************************************************************************/
/*	Driver's internal HW mutexes	*/
static xHOS_HardwareDelay_t pxHardwareDelayArr[configHOS_HARDWARE_DELAY_MAX_NUMBER_OF_OBJECTS];

/*	Number of so-far initialized objects	*/
static uint8_t ucHardwareDelayNumberOfCreatedObjects = 0;

/*******************************************************************************
 * Helping (private) functions:
 ******************************************************************************/
static inline void vInitHWTimer(	uint8_t ucTimerUnitNumber,
									xHOS_HardwareDelay_InitApproxFreq_t xFreqApproximate	)
{
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
xHOS_HardwareDelay_t* pxHOS_HardwareDelay_initNewObject(xHOS_HardwareDelay_InitApproxFreq_t xFreqApproximate)
{
	/*	check if there're available resources for new object	*/
	configASSERT(ucHardwareDelayNumberOfCreatedObjects < configHOS_HARDWARE_DELAY_MAX_NUMBER_OF_OBJECTS);

	/*	Init HW timer using port driver	*/
	uint8_t ucTimerUnitNumber = configHOS_HardwareDelayTimerUnits[ucHardwareDelayNumberOfCreatedObjects];
	vInitHWTimer(ucTimerUnitNumber, xFreqApproximate);

	/*	Init "xHOS_HardwareDelay_t" object parameters	*/
	xHOS_HardwareDelay_t* pxHandle = &pxHardwareDelayArr[ucHardwareDelayNumberOfCreatedObjects];
	vInitHandleParams(pxHandle, ucTimerUnitNumber, xFreqApproximate);

	/*	Init overflow interrupt in interrupt controller	*/
	vInitInterruptController(ucTimerUnitNumber);

	return pxHandle;
}

/*
 * See header file for info.
 */
void vHOS_HardwareDelay_delayTicks(xHOS_HardwareDelay_t* pxHandle, uint32_t uiTicks)
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
void vHOS_HardwareDelay_delayUs(xHOS_HardwareDelay_t* pxHandle, uint32_t uiTicks)
{

}

/*******************************************************************************
 * Overflow handlers:
 ******************************************************************************/
#define HANDLER(n)                                                                  \
void port_TIM_OVF_HANDLER_##n (void)     	                                        \
{                                                                                   \
	/*	Check that OVF of n-th timer unit is what have caused the interrupt	*/      \
	if (!ucPort_TIM_getOverflowFlag(n))                                             \
		return;                                                                     \
                                                                                    \
	/*	Stop counter and clear OVF flag	*/                                          \
	vPort_TIM_disableCounter(n);                                                    \
	vPort_TIM_clearOverflowFlag(n);                                                 \
                                                                                    \
	/*	Give HW mutex	*/                                                          \
	BaseType_t xHighPriorityTaskWoken = pdFALSE;                                    \
	xSemaphoreGiveFromISR(pxHardwareDelayArr[n].xHWMutex, &xHighPriorityTaskWoken); \
	portYIELD_FROM_ISR(xHighPriorityTaskWoken);                                     \
}

#if (configHOS_HARDWARE_DELAY_MAX_NUMBER_OF_OBJECTS > 0)
HANDLER(0)
#endif

#if (configHOS_HARDWARE_DELAY_MAX_NUMBER_OF_OBJECTS > 1)
HANDLER(1)
#endif

#if (configHOS_HARDWARE_DELAY_MAX_NUMBER_OF_OBJECTS > 2)
HANDLER(2)
#endif

#if (configHOS_HARDWARE_DELAY_MAX_NUMBER_OF_OBJECTS > 3)
HANDLER(3)
#endif

#if (configHOS_HARDWARE_DELAY_MAX_NUMBER_OF_OBJECTS > 4)
HANDLER(4)
#endif

#if (configHOS_HARDWARE_DELAY_MAX_NUMBER_OF_OBJECTS > 5)
HANDLER(5)
#endif

#if (configHOS_HARDWARE_DELAY_MAX_NUMBER_OF_OBJECTS > 6)
HANDLER(6)
#endif

#if (configHOS_HARDWARE_DELAY_MAX_NUMBER_OF_OBJECTS > 7)
HANDLER(7)
#endif

#if (configHOS_HARDWARE_DELAY_MAX_NUMBER_OF_OBJECTS > 8)
HANDLER(8)
#endif

#if (configHOS_HARDWARE_DELAY_MAX_NUMBER_OF_OBJECTS > 9)
HANDLER(9)
#endif
























