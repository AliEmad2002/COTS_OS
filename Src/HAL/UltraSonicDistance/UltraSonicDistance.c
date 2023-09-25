/*
 * UltraSonicDistance.c
 *
 *  Created on: Aug 29, 2023
 *      Author: Ali Emad
 */

/*	LIB	*/
#include <stdint.h>
#include <stdio.h>
#include "LIB/NAvgFilter/NAvgFilter.h"

/*	FreeRTOS	*/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/*	MCAL (Ported)	*/
#include "MCAL_Port/Port_DIO.h"
#include "MCAL_Port/Port_EXTI.h"
#include "MCAL_Port/Port_Interrupt.h"

/*	HAL-OS	*/
#include "RTOS_PRI_Config.h"
#include "HAL/HWTime/HWTime.h"

/*	SELF	*/
#include "HAL/UltraSonicDistance/UltraSonicDistance.h"

/*******************************************************************************
 * Helping functions/macros.
 ******************************************************************************/
static inline void vBlockUntilNextSampleTime(xHOS_UltraSonicDistance_t* pxHandle)
{
	if (pxHandle->ucIsConnectedToSynchronizer)
	{
		xSemaphoreTake(pxHandle->xSyncSemaphore, portMAX_DELAY);
	}
}


/*******************************************************************************
 * ISR callback:
 ******************************************************************************/
static inline void vRisingCallback(xHOS_UltraSonicDistance_t* pxHandle)
{
	/*	Update risingEchoTimestamp	*/
	pxHandle->risingEchoTimestamp = ulHOS_HWTime_getTimestampFromISR();

	/*	Configure EXTI to  generate event on falling edge	*/
	vPort_EXTI_setEdge(pxHandle->ucEchoPort, pxHandle->ucEchoPin, 0);
}

static inline void vFallingCallback(xHOS_UltraSonicDistance_t* pxHandle)
{
	/*	Update fallingEchoTimestamp	*/
	pxHandle->fallingEchoTimestamp = ulHOS_HWTime_getTimestampFromISR();

	/*
	 * If "fallingEchoTimestamp" is too close to "risingEchoTimestamp", then
	 * the signal which has caused the interrupt is rising edge noise.
	 *
	 * (Rising edge noise: Since the EXTI is capturing signal level at ABP bus
	 * frequency, which is much faster than low to high transition of most sensors,
	 * there would be a time slice where pin voltage is about Vdd/2. And because of
	 * Vdd noise (few milli-volts), a false falling edge may be detected).
	 */
	if (	pxHandle->fallingEchoTimestamp - pxHandle->risingEchoTimestamp <
			ulHOS_HWTime_US_TO_TICKS(10)	)
	{
		return;
	}

	/*	Disable interrupt (Enabled by handle's task at next sample)	*/
	vPORT_EXTI_DISABLE_LINE(pxHandle->ucEchoPort, pxHandle->ucEchoPin);

	/*	Acknowledge handle's task that a new read is available	*/
	BaseType_t xHighPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(	pxHandle->xNewReadSemaphore,
							&xHighPriorityTaskWoken	);

	portYIELD_FROM_ISR(xHighPriorityTaskWoken);
}

static void vCallback(void* pvParams)
{
	xHOS_UltraSonicDistance_t* pxHandle = (xHOS_UltraSonicDistance_t*)pvParams;

	if (ucPort_EXTI_getEdge(pxHandle->ucEchoPort, pxHandle->ucEchoPin) == 1)
	{
		vRisingCallback(pxHandle);
	}
	else
	{
		vFallingCallback(pxHandle);
	}
}

/*******************************************************************************
 * RTOS Task code:
 ******************************************************************************/
static void vTask(void* pvParams)
{
	xHOS_UltraSonicDistance_t* pxHandle = (xHOS_UltraSonicDistance_t*)pvParams;
	uint64_t ulDeltaTicks;
	int32_t iNewDist;
	TickType_t xLastWakeTime;

	while(1)
	{
		/*
		 * When the sensor handle is connected to a synchronizer handle, new sample
		 * does not start until synchronizer says so.
		 */
		vBlockUntilNextSampleTime(pxHandle);

		/*
		 * Store  starting time.
		 */
		xLastWakeTime = xTaskGetTickCount();

		/*	Assure EXTI semaphore is not available (force it)	*/
		xSemaphoreTake(pxHandle->xNewReadSemaphore, 0);

		/*	Configure EXTI to generate an event on the rising edge of echo signal	*/
		vPort_EXTI_setEdge(pxHandle->ucEchoPort, pxHandle->ucEchoPin, 1);

		/*	Clear EXTI flag	*/
		vPORT_EXTI_CLEAR_PENDING_FLAG(pxHandle->ucEchoPort, pxHandle->ucEchoPin);

		/*	Enable EXTI interrupt (Disabled in the ISR)	*/
		vPORT_EXTI_ENABLE_LINE(pxHandle->ucEchoPort, pxHandle->ucEchoPin);

		/*	trigger sensor	*/
		vPORT_DIO_WRITE_PIN(pxHandle->ucTrigPort, pxHandle->ucTrigPin, 1);

		/*	After 1 ms, turn off the trigger signal	*/
		vTaskDelay(1);
		vPORT_DIO_WRITE_PIN(pxHandle->ucTrigPort, pxHandle->ucTrigPin, 0);

		/*
		 * Block until EXTI callback is executed and has stored new timestamps,
		 * or until sample time passes.
		 */
		if (!xSemaphoreTake(
				pxHandle->xNewReadSemaphore,
				pdMS_TO_TICKS(1000ul / pxHandle->ucSamplesPerseconds)	))
		{
			/*	Do nothing	*/
		}

		else
		{
			/*	Calculate distance	*/
			ulDeltaTicks = pxHandle->fallingEchoTimestamp - pxHandle->risingEchoTimestamp;
			iNewDist = (1000 * (343/2) * ulDeltaTicks) / uiHOS_HWTIME_TIMER_FREQ_ACTUAL;

			/*	Add new distance to the filter	*/
			xSemaphoreTake(pxHandle->xFilter.xMutex, portMAX_DELAY);
			vLIB_NAvgFilter_update(&pxHandle->xFilter, iNewDist);
			xSemaphoreGive(pxHandle->xFilter.xMutex);
		}

		/*	Disable EXTI interrupt	*/
		vPORT_EXTI_DISABLE_LINE(pxHandle->ucEchoPort, pxHandle->ucEchoPin);

		/*	Delay until next sample time	*/
		vTaskDelayUntil(
			&xLastWakeTime,
			pdMS_TO_TICKS(1000ul / pxHandle->ucSamplesPerseconds)	);
	}
}

/*******************************************************************************
 * API functions:
 ******************************************************************************/
/*
 * See header file for info.
 */
void vHOS_UltraSonicDistance_init(	xHOS_UltraSonicDistance_t* pxHandle,
									uint32_t uiNFilter,
									int32_t piFilterMemory[uiNFilter]	)
{
	/*	Initialize task	*/
	static uint8_t ucCreatedObjectsCount = 0;
	char pcTaskName[configMAX_TASK_NAME_LEN];
	sprintf(pcTaskName, "USD%d", ucCreatedObjectsCount++);

	pxHandle->xTask = xTaskCreateStatic(	vTask,
											pcTaskName,
											configMINIMAL_STACK_SIZE,
											(void*)pxHandle,
											configHOS_SOFT_REAL_TIME_TASK_PRI,
											pxHandle->xTaskStack,
											&pxHandle->xTaskStatic	);

	/*	Initialize new read semaphore	*/
	pxHandle->xNewReadSemaphore =
		xSemaphoreCreateBinaryStatic(&pxHandle->xNewReadSemaphoreStatic);
	xSemaphoreTake(pxHandle->xNewReadSemaphore, 0);

	/*	Initialize sync semaphore (if connected to a synchronizer)	*/
	if (pxHandle->ucIsConnectedToSynchronizer)
	{
		pxHandle->xSyncSemaphore =
			xSemaphoreCreateBinaryStatic(&pxHandle->xSyncSemaphoreStatic);
		xSemaphoreTake(pxHandle->xSyncSemaphore, 0);
	}

	/*	Initialize filter	*/
	vLIB_NAvgFilter_init(&pxHandle->xFilter, piFilterMemory, uiNFilter);

	/*	Initialize pins	*/
	vPort_DIO_initPinOutput(pxHandle->ucTrigPort, pxHandle->ucTrigPin);
	vPORT_DIO_WRITE_PIN(pxHandle->ucTrigPort, pxHandle->ucTrigPin, 0);

	vPort_DIO_initPinInput(pxHandle->ucEchoPort, pxHandle->ucEchoPin, 0);

	/*	Initialize EXTI	*/
	vPort_EXTI_setEdge(pxHandle->ucEchoPort, pxHandle->ucEchoPin, 1);
	vPORT_EXTI_DISABLE_LINE(pxHandle->ucEchoPort, pxHandle->ucEchoPin);
	vPort_EXTI_setCallback(	pxHandle->ucEchoPort,
							pxHandle->ucEchoPin,
							vCallback,
							(void*)pxHandle	);

	/*	Initialize interrupt controller	*/
	VPORT_INTERRUPT_SET_PRIORITY(
		pxPortInterruptExtiIrqNumberArr[pxHandle->ucEchoPin],
		configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1	);

	vPORT_INTERRUPT_ENABLE_IRQ(pxPortInterruptExtiIrqNumberArr[pxHandle->ucEchoPin]);
}

/*
 * See header file for info.
 */
uint32_t uiHOS_UltraSonicDistance_getDist(xHOS_UltraSonicDistance_t* pxHandle)
{
	uint32_t uiDist;

	xSemaphoreTake(pxHandle->xFilter.xMutex, portMAX_DELAY);
	uiDist = pxHandle->xFilter.iAvg;
	xSemaphoreGive(pxHandle->xFilter.xMutex);

	return uiDist;
}
