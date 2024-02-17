/*
 * ADC.c
 *
 *  Created on: Dec 16, 2023
 *      Author: Ali Emad
 */

/*	LIB	*/
#include "stdint.h"

/*	RTOS	*/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/*	MCAL	*/
#include "MCAL_Port/Port_ADC.h"
#include "MCAL_Port/Port_Interrupt.h"

/*	SELF	*/
#include "HAL/ADC/ADC.h"

/*******************************************************************************
 * Helping structures:
 ******************************************************************************/
typedef struct{
	/*	Unit's mutex (Locked by the using task)	*/
	SemaphoreHandle_t xMutex;
	StaticSemaphore_t xMutexStatic;

	/*
	 * End-Of-Conversion semaphore. Given by EOC ISR, and taken by the task blocked
	 * on it.
	 */
	SemaphoreHandle_t xEocSemphr;
	StaticSemaphore_t xEocSemphrStatic;
}xHOS_ADC_t;

/*******************************************************************************
 * Static variables:
 ******************************************************************************/
static xHOS_ADC_t pxAdcArr[uiCONF_ADC_NUMBER_OF_UNITS];

/*******************************************************************************
 * Static helping functions:
 ******************************************************************************/

/*******************************************************************************
 * ISR callback:
 ******************************************************************************/
void vEocCallback(void* pvParams)
{
	uint8_t ucUnitNumber = (uint8_t)(uint32_t)pvParams;

	xHOS_ADC_t* pxHandle = &pxAdcArr[ucUnitNumber];

	BaseType_t xIsHptWoken = pdFALSE;
	xSemaphoreGiveFromISR(pxHandle->xEocSemphr, &xIsHptWoken);
	portYIELD_FROM_ISR(xIsHptWoken);
}

/*******************************************************************************
 * API functions:
 ******************************************************************************/
/*
 * See header for info.
 */
void vHOS_ADC_init(void)
{
	/*	Ignore more  than one call in the whole reset cycle	*/
	static uint8_t ucIsPrevInit = 0;

	if (ucIsPrevInit == 1)
		return;

	ucIsPrevInit = 1;

	/*	Start initialization	*/
	xHOS_ADC_t* pxHandle;

	for (uint8_t i = 0; i < uiCONF_ADC_NUMBER_OF_UNITS; i++)
	{
		pxHandle = &pxAdcArr[i];

		/*	Create unit's mutex	*/
		pxHandle->xMutex = xSemaphoreCreateMutexStatic(&pxHandle->xMutexStatic);
		xSemaphoreGive(pxHandle->xMutex);

		/*	Create unit's EOC semaphore	*/
		pxHandle->xEocSemphr = xSemaphoreCreateBinaryStatic(&pxHandle->xEocSemphrStatic);
		xSemaphoreTake(pxHandle->xEocSemphr, 0);

		/*	Initialize interrupt	*/
		vPort_ADC_setInterruptCallback(i, vEocCallback, (void*)(uint32_t)i);

		vPORT_ADC_ENABLE_EOC_INTERRUPT(i);

		uint32_t uiIrqN = pxPortInterruptAdcEocIrqNumberArr[i];

		VPORT_INTERRUPT_SET_PRIORITY(	uiIrqN,
										configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY	);

		vPORT_INTERRUPT_ENABLE_IRQ(uiIrqN);
	}
}

/*
 * See header for info.
 */
uint8_t ucHOS_ADC_lockUnit(uint8_t ucUnitNumber, TickType_t xTimeout)
{
	xHOS_ADC_t* pxHandle = &pxAdcArr[ucUnitNumber];

	return xSemaphoreTake(pxHandle->xMutex, xTimeout);
}

/*
 * See header for info.
 */
void vHOS_ADC_unlockUnit(uint8_t ucUnitNumber)
{
	xHOS_ADC_t* pxHandle = &pxAdcArr[ucUnitNumber];

	xSemaphoreGive(pxHandle->xMutex);
}

/*
 * See header for info.
 */
inline void vHOS_ADC_selectChannel(uint8_t ucUnitNumber, uint8_t ucChannelNumber)
{
	vPORT_ADC_SELECT_CHANNEL(ucUnitNumber, ucChannelNumber);
}

/*
 * See header for info.
 */
inline void vHOS_ADC_setSampleTime(	uint8_t ucUnitNumber,
		uint8_t ucChannelNumber,
		uint32_t uiSampleTime	)
{
	vPort_ADC_setChannelSampleTime(ucUnitNumber, ucChannelNumber, uiSampleTime);
}

/*
 * See header for info.
 */
inline void vHOS_ADC_setTriggerSource(uint8_t ucUnitNumber, uint8_t ucSrc)
{
	vPort_ADC_setTriggerSource(ucUnitNumber, ucSrc);
}

/*
 * See header for info.
 */
inline void vHOS_ADC_triggerRead(uint8_t ucUnitNumber)
{
	vPORT_ADC_TRIGGER_CONVERSION(ucUnitNumber);
}

/*
 * See header for info.
 */
inline void vHOS_ADC_selectMode(uint8_t ucUnitNumber, uint8_t ucMode)
{
	vPort_ADC_setConversionMode(ucUnitNumber, ucMode);
}

/*
 * See header for info.
 */
uint8_t ucHOS_ADC_blockUntilEOC(uint8_t ucUnitNumber, TickType_t xTimeout)
{
	xHOS_ADC_t* pxHandle = &pxAdcArr[ucUnitNumber];

	uint8_t ucReturnVal = xSemaphoreTake(pxHandle->xEocSemphr, xTimeout);

	return ucReturnVal;
}

/*
 * See header for info.
 */
inline uint32_t uiHOS_ADC_read(uint8_t ucUnitNumber)
{
	return usPORT_ADC_GET_DR(ucUnitNumber);
}

/*
 * See header for info.
 */
uint32_t uiHOS_ADC_readChannelBlocking(uint8_t ucUnitNumber, uint8_t ucChannelNumber)
{
	/*	Select channel as ADC input	*/
	vHOS_ADC_selectChannel(ucUnitNumber, ucChannelNumber);

	/*	Start ADC conversion	*/
	vHOS_ADC_triggerRead(ucUnitNumber);

	/*	Block until EOC	*/
	ucHOS_ADC_blockUntilEOC(ucUnitNumber, portMAX_DELAY);

	/*	Get value	*/
	uint32_t uiVal = usPORT_ADC_GET_DR(ucUnitNumber);

	return uiVal;
}

/*
 * See header for info.
 */
int32_t iHOS_ADC_getVoltageCalib(int32_t iRawRead, uint32_t uiVrefIntRead)
{
	return ((int64_t)iRawRead * (uiPORT_ADC_VREFINT_IN_MV * 1000)) / uiVrefIntRead;
}

/*
 * See header for info.
 */
inline uint32_t uiHOS_ADC_getVoltageDirect(uint32_t uiRawRead)
{
	return ((uint64_t)uiRawRead * (uiPORT_ADC_VREF_IN_MV * 1000)) / 4096;
}








