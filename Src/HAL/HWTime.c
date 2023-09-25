/*
 * HWTime.c
 *
 *  Created on: Aug 28, 2023
 *      Author: Ali Emad
 */

/*	LIB	*/
#include <stdint.h>
#include "LIB/Heap/Heap.h"

/*	FreeRTOS	*/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "RTOS_PRI_Config.h"

/*	MCAL (Ported)	*/
#include "MCAL_Port/Port_Timer.h"
#include "MCAL_Port/Port_Interrupt.h"

/*	SELF	*/
#include "HAL/HWTime/HWTime.h"

#define uiCONF_HWTIME_MAX_NUMBER_OF_DELAYS		50

/*******************************************************************************
 * Static / Global variables:
 ******************************************************************************/
static uint32_t uiOvfCount = 0;

static uint32_t uiMaxCounterVal;

static xLIB_Heap_t xHeap;

static StaticTask_t xTaskStatic;
static TaskHandle_t xTask;
static StackType_t xTaskStack[configMINIMAL_STACK_SIZE];

static SemaphoreHandle_t xCaptureCompareSemaphore;
static StaticSemaphore_t xCaptureCompareSemaphoreStatic;

static SemaphoreHandle_t xConfigCountingSemaphore;
static StaticSemaphore_t xConfigCountingSemaphoreStatic;

static SemaphoreHandle_t xHeapSizeSemaphore;
static StaticSemaphore_t xHeapSizeSemaphoreStatic;

/*******************************************************************************
 * Callbacks:
 ******************************************************************************/
static void vOvfCallback(void* pvParams)
{
	/*	Increment overflow counter	*/
	uiOvfCount++;
}

static void vCcCallback(void* pvParams)
{
	vPort_TIM_disableCcInterrupt(ucHOS_HWTIME_TIMER_UNIT_NUMBER);

	BaseType_t xHighPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(	xCaptureCompareSemaphore,
							&xHighPriorityTaskWoken	);

	portYIELD_FROM_ISR(xHighPriorityTaskWoken);
}

/*******************************************************************************
 * RTOS task function:
 ******************************************************************************/
static void vTask(void* pvParams)
{
	SemaphoreHandle_t* pxCurrentRunningDelaySemaphore = NULL;

	xHOS_HWTime_Delay_t* pxHeapTop;

	while(1)
	{
		/*	If compare unit is currently configured for a delay	*/
		if (xSemaphoreTake(xConfigCountingSemaphore, 0))
		{
			/*	Block until capture/compare interrupt occurs	*/
			xSemaphoreTake(xCaptureCompareSemaphore, portMAX_DELAY);

			/*	Give semaphore of the just ended delay	*/
			xSemaphoreGive(*pxCurrentRunningDelaySemaphore);
			//taskYIELD();
		}

		/*
		 * Block until heap has elements in it.
		 */
		xSemaphoreTake(xHeapSizeSemaphore, portMAX_DELAY);
		xSemaphoreGive(xHeapSizeSemaphore);

		/*	Lock the heap	*/
		ucLIB_HEAP_LOCK(&xHeap, portMAX_DELAY);

		/*	Get pointer to heap's top delay handle	*/
		pxHeapTop = (xHOS_HWTime_Delay_t*)pxLIB_HEAP_GET_TOP_VAL(&xHeap);

		/*
		 * If heap's top time has already passed, release its semaphore and remove
		 * it from the heap.
		 */
		if (pxHeapTop->ulReleaseTimestamp <= ulHOS_HWTime_getTimestamp())
		{
			xSemaphoreGive(pxHeapTop->xSemaphore);
			//taskYIELD();

			vLIB_Heap_removeTop(&xHeap);
			xSemaphoreTake(xHeapSizeSemaphore, 0);
		}

		/*
		 * Otherwise, if heap's top time has not yet passed and is in the current
		 * OVF iteration.
		 */
		else if (pxHeapTop->ulReleaseTimestamp / uiMaxCounterVal == uiOvfCount)
		{
			/*	Configure HW timer compare unit with it	*/
			vPort_TIM_writeOcRegister(
				ucHOS_HWTIME_TIMER_UNIT_NUMBER,
				pxHeapTop->ulReleaseTimestamp % uiMaxCounterVal	);

			xSemaphoreGive(xConfigCountingSemaphore);
			vPORT_TIM_CLEAR_CC_FLAG(ucHOS_HWTIME_TIMER_UNIT_NUMBER);
			vPort_TIM_enableCcInterrupt(ucHOS_HWTIME_TIMER_UNIT_NUMBER);

			/*	If heap's top time has passed while being configured	*/
			if (pxHeapTop->ulReleaseTimestamp <= ulHOS_HWTime_getTimestamp())
			{
				/*	Give its semaphore immediately	*/
				xSemaphoreGive(pxHeapTop->xSemaphore);
				//taskYIELD();

				/*	De-configure HW timer compare unit.	*/
				vPort_TIM_disableCcInterrupt(ucHOS_HWTIME_TIMER_UNIT_NUMBER);
				vPORT_TIM_CLEAR_CC_FLAG(ucHOS_HWTIME_TIMER_UNIT_NUMBER);
				xSemaphoreTake(xCaptureCompareSemaphore, 0);
				xSemaphoreTake(xConfigCountingSemaphore, 0);
			}

			/*
			 * Otherwise, store pointer to its semaphore to be released when delay
			 * ends.
			 */
			pxCurrentRunningDelaySemaphore = &pxHeapTop->xSemaphore;

			/*	Remove heap's top	*/
			vLIB_Heap_removeTop(&xHeap);
			xSemaphoreTake(xHeapSizeSemaphore, portMAX_DELAY);
		}

		/*	Release the heap	*/
		vLIB_HEAP_RELEASE(&xHeap);
	}
}

/*******************************************************************************
 * Helping functions / macros:
 ******************************************************************************/
static inline void vInitHWTimer(void)
{
	uint8_t ucTimerUnitNumber = ucHOS_HWTIME_TIMER_UNIT_NUMBER;

	uiMaxCounterVal =
		(1ul << pucPortTimerCounterSizeInBits[ucHOS_HWTIME_TIMER_UNIT_NUMBER]) - 1;

	vPort_TIM_disableCounter(ucTimerUnitNumber);

	vPort_TIM_useInternalClockSource(ucTimerUnitNumber);

	vPort_TIM_setPrescaler(ucTimerUnitNumber, uiHOS_HWTIME_TIMER_PRESCALER);

	vPort_TIM_setModeNormal(ucTimerUnitNumber);

	vPORT_TIM_CLEAR_OVF_FLAG(ucTimerUnitNumber);

	vPort_TIM_setOvfCallback(ucTimerUnitNumber, vOvfCallback, NULL);

	vPort_TIM_setCcCallback(ucTimerUnitNumber, vCcCallback, NULL);

	vPort_TIM_enableOverflowInterrupt(ucTimerUnitNumber);

	vPort_TIM_setCountingDirUp(ucTimerUnitNumber);

	vPort_TIM_setCounterUpperLimit(ucTimerUnitNumber, uiMaxCounterVal);

	vPort_TIM_enableCounter(ucTimerUnitNumber);
}

static inline void vInitInterruptController()
{
	uint8_t ucTimerUnitNumber = ucHOS_HWTIME_TIMER_UNIT_NUMBER;

	vPort_Interrupt_setPriority(
		pxPortInterruptTimerOvfIrqNumberArr[ucTimerUnitNumber],
		configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);

	vPort_Interrupt_enableIRQ(pxPortInterruptTimerOvfIrqNumberArr[ucTimerUnitNumber]);

	vPort_Interrupt_setPriority(
		pxPortInterruptTimerCcIrqNumberArr[ucTimerUnitNumber],
		configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);

	vPort_Interrupt_enableIRQ(pxPortInterruptTimerCcIrqNumberArr[ucTimerUnitNumber]);
}

static inline void vInitDelayController()
{
	/*	Initialize the heap	*/
	vLIB_Heap_init(&xHeap);

	/*	Initialize capture/compare semaphore	*/
	xCaptureCompareSemaphore = xSemaphoreCreateBinaryStatic(&xCaptureCompareSemaphoreStatic);
	xSemaphoreTake(xCaptureCompareSemaphore, 0);

	/*	Initialize configuration counting semaphore	*/
	xConfigCountingSemaphore = xSemaphoreCreateCountingStatic(
		uiCONF_HWTIME_MAX_NUMBER_OF_DELAYS,
		0,
		&xConfigCountingSemaphoreStatic);

	/*	Initialize heap size counting semaphore	*/
	xHeapSizeSemaphore = xSemaphoreCreateCountingStatic(
		uiCONF_HWTIME_MAX_NUMBER_OF_DELAYS,
		0,
		&xHeapSizeSemaphoreStatic);

	/*	Initialize task	*/
	xTask = xTaskCreateStatic(	vTask,
								"HWTime",
								configMINIMAL_STACK_SIZE,
								NULL,
								configHOS_HWTIME_TASK_PRI,
								xTaskStack,
								&xTaskStatic	);
}

static inline ucIsThereActiveDelay(void)
{
	uint8_t ucIsThereActiveDelay = xSemaphoreTake(xConfigCountingSemaphore, 0);
	if (ucIsThereActiveDelay)
		xSemaphoreGive(xConfigCountingSemaphore);

	return ucIsThereActiveDelay;
}

/*******************************************************************************
 * API functions:
 ******************************************************************************/
/*
 * See header for info.
 */
void vHOS_HWTime_init(void)
{
	/*	Initialize HW timer	*/
	vInitHWTimer();

	/*	Initialize interrupt	*/
	vInitInterruptController();

	/*	Initialize delay controller SW	*/
	vInitDelayController();
}

/*
 * See header for info.
 */
uint64_t ulHOS_HWTime_getTimestamp(void)
{
	//return uiOvfCount * uiMaxCounterVal + uiPORT_TIM_READ_COUNTER(ucHOS_HWTIME_TIMER_UNIT_NUMBER);

	uint64_t ulTimestamp;
	register uint32_t uiCNT;
	register uint32_t uiOvfFlag;
	register uint32_t uiOvfOccuredBeforeCnt;

	taskENTER_CRITICAL();
	{
		/*	Read counter value and OVF flag	*/
		uiCNT = uiPORT_TIM_READ_COUNTER(ucHOS_HWTIME_TIMER_UNIT_NUMBER);
		uiOvfFlag = ucPORT_TIM_GET_OVF_FLAG(ucHOS_HWTIME_TIMER_UNIT_NUMBER);

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
uint64_t ulHOS_HWTime_getTimestampFromISR(void)
{
	/*
	 * Same implementation as the previous function, only difference is that this
	 * function uses the ISR-safe version of entering and exiting critical section.
	 */
	//return uiOvfCount * uiMaxCounterVal + uiPORT_TIM_READ_COUNTER(ucHOS_HWTIME_TIMER_UNIT_NUMBER);

	uint64_t ulTimestamp;
	register uint32_t uiCNT;
	register uint32_t uiOvfFlag;
	register uint32_t uiOvfOccuredBeforeCnt;

	UBaseType_t uxSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR();
	{
		uiCNT = uiPORT_TIM_READ_COUNTER(ucHOS_HWTIME_TIMER_UNIT_NUMBER);
		uiOvfFlag = ucPORT_TIM_GET_OVF_FLAG(ucHOS_HWTIME_TIMER_UNIT_NUMBER);
		uiOvfOccuredBeforeCnt = (uiCNT < uiMaxCounterVal/2) && uiOvfFlag;

		if (uiOvfOccuredBeforeCnt)
			ulTimestamp = (uiOvfCount + 1) * uiMaxCounterVal + uiCNT;
		else
			ulTimestamp = uiOvfCount * uiMaxCounterVal + uiCNT;
	}
	taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus);



	return ulTimestamp;

}

/*
 * See header for info.
 */
void vHOS_HWTime_initDelay(xHOS_HWTime_Delay_t* pxHandle)
{
	/*	Create semaphore	*/
	pxHandle->xSemaphore = xSemaphoreCreateBinaryStatic(&pxHandle->xSemaphoreStatic);

	/*	Initially available	*/
	xSemaphoreGive(pxHandle->xSemaphore);
}

/*
 * See header for info.
 */
void vHOS_HWTime_delay(xHOS_HWTime_Delay_t* pxHandle, uint32_t uiTicks)
{
	/*	Calculate time at which delay ends	*/
	pxHandle->ulReleaseTimestamp = ulHOS_HWTime_getTimestamp() + uiTicks;

	/*	If there's a currently active configured delay	*/
	if (ucIsThereActiveDelay())
	{
		/*	If new delay ends before the currently configured delay	*/
		//TODO
	}


	/*	Assure delay's semaphore is not available	*/
	(void)xSemaphoreTake(pxHandle->xSemaphore, 0);

	/*	Add to the heap	*/
	ucLIB_HEAP_LOCK(&xHeap, portMAX_DELAY);

	int32_t iHandleAddress = (int32_t)pxHandle;
	vLIB_Heap_addElem(&xHeap, &iHandleAddress);

	xSemaphoreGive(xHeapSizeSemaphore);

	vLIB_HEAP_RELEASE(&xHeap);

	/*	Lock until semaphore is released	*/
	xSemaphoreTake(pxHandle->xSemaphore, portMAX_DELAY);

	/*	Give it back (To notify other functions that delay has ended)	*/
	xSemaphoreGive(pxHandle->xSemaphore);
}

















