/*
 * UsbCdc.c
 *
 *  Created on: Sep 11, 2023
 *      Author: Ali Emad
 */

/*	LIB	*/
#include <stdint.h>
#include <stdio.h>

/*	FreeRTOS	*/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/*	MCAL (Ported)	*/
#include "MCAL_Port/Port_USB.h"

/*	Third-party dependencies	*/
#include "tusb.h"

/*	COTS_OS	*/
#include "RTOS_PRI_Config.h"

/*	SELF	*/
#include "HAL/UsbCdc/UsbCdc_Config.h"
#include "HAL/UsbCdc/UsbCdc.h"



/*******************************************************************************
 * Helping macros:
 ******************************************************************************/
#define uiTASK_STACK_SIZE		(configMINIMAL_STACK_SIZE)


/*******************************************************************************
 * Helping structures:
 ******************************************************************************/
/*	USB CDC unit	*/
typedef struct{
	SemaphoreHandle_t xTransmissionMutex;
	StaticSemaphore_t xTransmissionMutexStatic;

	SemaphoreHandle_t xReceptionMutex;
	StaticSemaphore_t xReceptionMutexStatic;
}xUsbCdc_t;

/*******************************************************************************
 * Driver's static variables:
 ******************************************************************************/
static xUsbCdc_t pxUnitArr[ucCONF_USB_CDC_NUMBER_OF_UNITS];


/*******************************************************************************
 * Helping functions:
 ******************************************************************************/
/*	Checks whether there are a new received data or not in a given tim window	*/
static uint8_t ucIsDataAvailable(uint8_t ucUnitNumber, TickType_t xTimeout)
{
	/*	Calculate end time	*/
	TickType_t xEndTime = xTaskGetTickCount() + xTimeout;

	/*	If overflow occurred	*/
	if (xEndTime < xTimeout)
		xEndTime = portMAX_DELAY;

	/*	While end time has not yet been reached	*/
	while (xTaskGetTickCount() < xEndTime)
	{
		/*	If data are available, return 1	*/
		if (tud_cdc_n_available(ucUnitNumber))
			return 1;
	}

	/*	No data are available, return 0	*/
	return 0;
}


/*******************************************************************************
 * Callbacks:
 ******************************************************************************/


/*******************************************************************************
 * API functions:
 ******************************************************************************/
/*
 * See header for info.
 */
void vHOS_UsbCdc_init(void)
{
	xUsbCdc_t* pxUnit;

	for (uint8_t i = 0; i < ucCONF_USB_CDC_NUMBER_OF_UNITS; i++)
	{
		pxUnit = &pxUnitArr[i];

		/*	Initialize USB transmission mutex	*/
		pxUnit->xTransmissionMutex = xSemaphoreCreateMutexStatic(
			&pxUnit->xTransmissionMutexStatic	);

		xSemaphoreGive(pxUnit->xTransmissionMutex);

		/*	Initialize USB reception mutex	*/
		pxUnit->xReceptionMutex = xSemaphoreCreateMutexStatic(
			&pxUnit->xReceptionMutexStatic	);

		xSemaphoreGive(pxUnit->xReceptionMutex);
	}
}

/*
 * See header for info.
 */
uint8_t ucHOS_UsbCdc_lockTransmission(uint8_t ucUnitNumber, TickType_t xTimeout)
{
	xUsbCdc_t* pxHandle = &pxUnitArr[ucUnitNumber];

	return xSemaphoreTake(pxHandle->xTransmissionMutex, xTimeout);
}

/*
 * See header for info.
 */
void vHOS_UsbCdc_releaseTransmission(uint8_t ucUnitNumber)
{
	xUsbCdc_t* pxHandle = &pxUnitArr[ucUnitNumber];

	xSemaphoreGive(pxHandle->xTransmissionMutex);
}

/*
 * See header for info.
 */
uint8_t ucHOS_UsbCdc_lockReception(uint8_t ucUnitNumber, TickType_t xTimeout)
{
	xUsbCdc_t* pxHandle = &pxUnitArr[ucUnitNumber];

	return xSemaphoreTake(pxHandle->xReceptionMutex, xTimeout);
}

/*
 * See header for info.
 */
void vHOS_UsbCdc_releaseReception(uint8_t ucUnitNumber)
{
	xUsbCdc_t* pxHandle = &pxUnitArr[ucUnitNumber];

	xSemaphoreGive(pxHandle->xReceptionMutex);
}

/*
 * See header for info.
 */
void vHOS_UsbCdc_send(uint8_t ucUnitNumber, uint8_t* pucBuffer, uint16_t usLen)
{
	tud_cdc_n_write(ucUnitNumber, (void*)pucBuffer, usLen);
	tud_cdc_n_write_flush(ucUnitNumber);
}

/*
 * See header for info.
 */
uint8_t ucHOS_UsbCdc_readRxBuffer(	uint8_t ucUnitNumber,
									uint8_t* pucBuffer,
									uint32_t* puiLen,
									TickType_t xTimeout	)
{
	/*	Wait for  new data to be available, or timeout to be done	*/
	uint8_t ucIsNewDataAvailable = ucIsDataAvailable(ucUnitNumber, xTimeout);

	/*	If no new data is available, return 0	*/
	if (!ucIsNewDataAvailable)
		return 0;

	/*	Otherwise, copy the RxBuffer to "pucBuffer"	*/
	*puiLen = tud_cdc_n_read(ucUnitNumber, (void*)pucBuffer, 0xFFFFFFFF);

	/*	Read successful	*/
	return 1;
}

/*
 * See header for info.
 */
uint8_t ucHOS_UsbCdc_receive(	uint8_t ucUnitNumber,
								uint8_t* pucBuffer,
								uint32_t uiLen,
								TickType_t xTimeout	)
{
	TickType_t xCurrentTime = xTaskGetTickCount();
	TickType_t xEndTime = xTimeout + xCurrentTime;
	if (xEndTime < xCurrentTime)
		xEndTime = portMAX_DELAY;

	uint32_t uiCount = 0;
	uint32_t uiLenTemp;

	while(1)
	{
		if (!ucHOS_UsbCdc_readRxBuffer(	ucUnitNumber,
										&pucBuffer[uiCount],
										&uiLenTemp,
										xEndTime - xCurrentTime	)	)
			return 0;

		uiCount += uiLenTemp;

		if (uiCount >= uiLen)
			return 1;

		xCurrentTime = xTaskGetTickCount();
		if (xCurrentTime > xEndTime)
			return 0;
	}
}















