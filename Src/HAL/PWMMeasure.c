/*
 * PWMMeasure.c
 *
 *  Created on: Sep 20, 2023
 *      Author: Ali Emad
 */


/*	LIB	*/
#include <stdint.h>
#include <stdio.h>

/*	RTOS	*/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "RTOS_PRI_Config.h"

/*	MCAL	*/
#include "MCAL_Port/Port_DIO.h"
#include "MCAL_Port/Port_EXTI.h"
#include "MCAL_Port/Port_Interrupt.h"

/*	HAL	*/
#include "HAL/HWTime/HWTime.h"

/*	SELF	*/
#include "HAL/PWMMeasure/PWMMeasure.h"


/*******************************************************************************
 * Helping functions / macros:
 ******************************************************************************/


/*******************************************************************************
 * Callbacks:
 ******************************************************************************/
static void vCallback(void* pvParams)
{
	xHOS_PWMDutyMeasure_t* pxHandle = (xHOS_PWMDutyMeasure_t*)pvParams;

	/*	Get current timestamp	*/
	uint64_t ulCurrentTime = ulHOS_HWTime_getTimestampFromISR();

	/*	Get pin state	*/
	uint8_t ucPinState = ucPort_DIO_readPin(pxHandle->ucPort, pxHandle->ucPin);

	/*	If callback is due to a rising edge	*/
	if (ucPinState == 1)
	{
		/*
		 * If previous detected edge was rising, then a falling edge was missed
		 * (Due to noise). Skip current edge for keeping a correct measurement.
		 */
		if (pxHandle->ucPrevEdge == 1)		return;

		pxHandle->ucPrevEdge = 1;

		/*	Calculate period (Time since  the previous rising edge	*/
		pxHandle->uiPeriodTime = ulCurrentTime - pxHandle->ulPrevRisingTime;
		pxHandle->ulPrevRisingTime = ulCurrentTime;
	}

	/*	Otherwise, if callback is due to a falling edge	*/
	else
	{
		/*
		 * If previous detected edge was falling, then a rising edge was missed
		 * (Due to noise). Skip current edge for keeping a correct measurement.
		 */
		if (pxHandle->ucPrevEdge == 0)		return;

		pxHandle->ucPrevEdge = 0;

		/*	Calculate active time (Time since  the previous rising edge	*/
		pxHandle->uiActiveTime = ulCurrentTime - pxHandle->ulPrevRisingTime;
	}

	/*	Acknowledge handle's task that a new measurement has been made	*/
	(void)xSemaphoreGiveFromISR(pxHandle->xNewMeasurementSemaphore, NULL);
}

/*******************************************************************************
 * RTOS task:
 ******************************************************************************/
/*
 * This task completes driver's functionality, by measuring an edge case which
 * can not be measured by the EXTI. That is, when duty cycle is either 0% or 100%
 */
static void vTask(void* pvParams)
{
	xHOS_PWMDutyMeasure_t* pxHandle = (xHOS_PWMDutyMeasure_t*)pvParams;

	uint32_t uiTimeoutMs;

	while(1)
	{
		/*
		 * Read the "newMeasurementSemaphore" with a timeout of:
		 * 		-	max{ 2 * periodTime, 1ms}.
		 * 		-	if signal period time is maximum (frequency equals zero),
		 * 			block the task until new read happens).
		 */
		if (pxHandle->uiPeriodTime == (uint32_t)-1)
			uiTimeoutMs = portMAX_DELAY;
		else
		{
			uiTimeoutMs = ulHOS_HWTime_TICKS_TO_MS(2 * pxHandle->uiPeriodTime);

			if (uiTimeoutMs < 1)
				uiTimeoutMs = 1;
		}

		/*
		 * Has the signal been idle (no edges) in that two periodic times?
		 */
		if (xSemaphoreTake(pxHandle->xNewMeasurementSemaphore, uiTimeoutMs) == 0)
		{
			if (ucPort_DIO_readPin(pxHandle->ucPort, pxHandle->ucPin))
				pxHandle->uiActiveTime = (uint32_t)-1; // maximum active time.
			else
				pxHandle->uiActiveTime = 0;

			/*	Has the signal been idle for N cycles, write its frequency to zero	*/
			pxHandle->uiNumberOfIdlePeriods++;
			if (pxHandle->uiNumberOfIdlePeriods == uiCONF_PWM_MEASURE_NUMBR_OF_IDLE_PEERIODS_TILL_ZERO_HZ)
				pxHandle->uiPeriodTime = (uint32_t)-1;
		}

		else
			pxHandle->uiNumberOfIdlePeriods = 0;
	}
}

/*******************************************************************************
 * API functions:
 ******************************************************************************/
/*
 * See header for info.
 */
void vHOS_PWMMeasure_init(xHOS_PWMDutyMeasure_t* pxHandle)
{
	/*	Initialize task	*/
	static uint8_t ucCreatedObjectsCount = 0;
	char pcTaskName[configMAX_TASK_NAME_LEN];
	sprintf(pcTaskName, "PWMM%d", ucCreatedObjectsCount++);

	pxHandle->xTask = xTaskCreateStatic(	vTask,
											pcTaskName,
											configMINIMAL_STACK_SIZE,
											(void*)pxHandle,
											uiCONF_PWM_MEASURE_RTOS_PRI,
											pxHandle->pxTaskStack,
											&pxHandle->xTaskStatic	);

	/*	Initialize new measurement semaphore	*/
	pxHandle->xNewMeasurementSemaphore =
		xSemaphoreCreateBinaryStatic(&pxHandle->xNewMeasurementSemaphoreStatic);

	(void)xSemaphoreTake(pxHandle->xNewMeasurementSemaphore, 0);

	/*	Initialize handle's variables	*/
	pxHandle->uiPeriodTime = 0;
	pxHandle->uiActiveTime = 0;
	pxHandle->ucPrevEdge = 2; 	//	Neither falling nor rising, so that the EXTI
								//	callback does not get stuck at  first edge.
	pxHandle->ulPrevRisingTime = 0;
	pxHandle->uiNumberOfIdlePeriods = 0;

	/*	Initialize pin	*/
	vPort_DIO_initPinInput(pxHandle->ucPort, pxHandle->ucPin, 0);

	/*	Initialize EXTI	*/
	vPort_EXTI_setEdge(pxHandle->ucPort, pxHandle->ucPin, 2);

	vPort_EXTI_setCallback(	pxHandle->ucPort,
							pxHandle->ucPin,
							vCallback,
							(void*)pxHandle	);

	vPort_EXTI_enableLine(pxHandle->ucPort, pxHandle->ucPin);

	/*	Initialize interrupt controller	*/
	uint32_t uiIrqNum = uiPort_EXTI_getIrqNum(pxHandle->ucPort, pxHandle->ucPin);

	vPort_Interrupt_setPriority(uiIrqNum, uiCONF_PWM_MEASURE_EXTI_PRI);

	vPort_Interrupt_enableIRQ(uiIrqNum);
}

/*
 * See header for info.
 */
uint32_t uiHOS_PWMMeasure_getDuty(xHOS_PWMDutyMeasure_t* pxHandle)
{
#define uiMAX_DUTY		((uint64_t)((uint32_t)-1))

	uint32_t uiDuty;

	/*
	 * Disable EXTI, to assure a new measurement won't take place in the middle
	 * of reading handle's data.
	 */
	vPort_EXTI_disableLine(pxHandle->ucPort, pxHandle->ucPin);

	uint32_t uiActiveTime = pxHandle->uiActiveTime;
	uint32_t uiPeriodTime = pxHandle->uiPeriodTime;

	/*	Enable EXTI back	*/
	vPort_EXTI_enableLine(pxHandle->ucPort, pxHandle->ucPin);

	/*	If active time is set to maximum value, or period time is zero, return maximum duty	*/
	if (uiActiveTime == (uint32_t)-1 || uiPeriodTime == 0)
		uiDuty = (uint32_t)-1;

	/*	Otherwise, calculate duty	*/
	else
		uiDuty = (uiMAX_DUTY * (uint64_t)uiActiveTime) / uiPeriodTime;

	return uiDuty;

#undef uiMAX_DUTY
}

/*
 * See header for info.
 */
uint32_t uiHOS_PWMMeasure_getFreq(xHOS_PWMDutyMeasure_t* pxHandle)
{
	uint32_t uiPeriodTime = pxHandle->uiPeriodTime;

	if (uiPeriodTime == 0)
		return (uint32_t)-1;

	else if (uiPeriodTime == (uint32_t)-1)
		return 0;

	else
		return 1000000 / ulHOS_HWTime_TICKS_TO_US(uiPeriodTime);
}












