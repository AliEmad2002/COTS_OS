/*
 * USBVirtualCOM.c
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

/*	COTS_OS	*/
#include "RTOS_PRI_Config.h"

/*	SELF	*/
#include "HAL/USBVirtualCOM/USBVirtualCOM.h"


/*******************************************************************************
 * Helping structures:
 ******************************************************************************/
typedef struct{
	uint8_t* pucBuffer;
	uint16_t usLen;
	uint8_t ucIsPending;
}xTxPend_t;

/*******************************************************************************
 * Driver's static variables:
 ******************************************************************************/
static SemaphoreHandle_t xUsbMutex;
static StaticSemaphore_t xUsbMutexStatic;

static SemaphoreHandle_t xTxPendSemaphore;
static StaticSemaphore_t xTxPendSemaphoreStatic;

static SemaphoreHandle_t xRxAvailableSemaphore;
static StaticSemaphore_t xRxAvailableSemaphoreStatic;

static StaticTask_t xTaskStatic;
static TaskHandle_t xTask;
static StackType_t xTaskStack[configMINIMAL_STACK_SIZE];

static xTxPend_t xTxPend;

/*******************************************************************************
 * RTOS task:
 ******************************************************************************/
static void vTask(void* pvParams)
{
	uint8_t ucIsUsbBusy;

	while(1)
	{
		/*	Check if there's a new pending Tx operation	*/
		if (xTxPend.ucIsPending)
		{
			/*	Try sending the pending Tx operation	*/
			ucIsUsbBusy = (ucPort_USB_send(xTxPend.pucBuffer, xTxPend.usLen) == 0);

			/*	If USB is busy, try again on next iteration	*/
			if (ucIsUsbBusy)
			{	}

			/*	Otherwise, clear the pending flag and give the TxPend semaphore	*/
			else
			{
				xTxPend.ucIsPending = 0;
				xSemaphoreGive(xTxPendSemaphore);
			}
		}

		/*	Check if there's new received data	*/
		if (*puiPortUsbRxBufferLen > 0)
		{
			/*	Release RxAvailable semaphore	*/
			xSemaphoreGive(xRxAvailableSemaphore);
		}

		/*	Delay for 1ms between iterations	*/
		vTaskDelay(pdMS_TO_TICKS(1));
	}
}

/*******************************************************************************
 * API functions:
 ******************************************************************************/
/*
 * See header for info.
 */
void vHOS_USBVirtualCOM_init(void)
{
	/*	Initialize USB mutex	*/
	xUsbMutex = xSemaphoreCreateMutexStatic(&xUsbMutexStatic);
	xSemaphoreGive(xUsbMutex);

	/*	Initialize TxPend semaphore	*/
	xTxPendSemaphore = xSemaphoreCreateBinaryStatic(&xTxPendSemaphoreStatic);
	xSemaphoreGive(xTxPendSemaphore);

	/*	Initialize RxAvailable semaphore	*/
	xRxAvailableSemaphore = xSemaphoreCreateBinaryStatic(&xRxAvailableSemaphoreStatic);
	xSemaphoreTake(xRxAvailableSemaphore, 0);

	/*	Initialize task	*/
	xTask = xTaskCreateStatic(	vTask,
								"USB",
								configMINIMAL_STACK_SIZE,
								NULL,
								configHOS_SOFT_REAL_TIME_TASK_PRI,
								xTaskStack,
								&xTaskStatic	);
}

/*
 * See header for info.
 */
uint8_t ucHOS_USBVirtualCOM_lockDriver(TickType_t xTimeout)
{
	return xSemaphoreTake(xUsbMutex, xTimeout);
}

/*
 * See header for info.
 */
void vHOS_USBVirtualCOM_releaseDriver(void)
{
	xSemaphoreGive(xUsbMutex);
}

/*
 * See header for info.
 */
void vHOS_USBVirtualCOM_blockUntilTxDone(void)
{
	/*	Wait for TxPend to be available	*/
	xSemaphoreTake(xTxPendSemaphore, portMAX_DELAY);
	xSemaphoreGive(xTxPendSemaphore);
}

/*
 * See header for info.
 */
void vHOS_USBVirtualCOM_send(uint8_t* pucBuffer, uint16_t usLen)
{
	/*	Wait for TxPend to be available	*/
	xSemaphoreTake(xTxPendSemaphore, portMAX_DELAY);

	/*	Pend new Tx data	*/
	xTxPend.pucBuffer = pucBuffer;
	xTxPend.usLen = usLen;
	xTxPend.ucIsPending = 1;
}

/*
 * See header for info.
 */
uint8_t ucHOS_USBVirtualCOM_readRxBuffer(	uint8_t* pucBuffer,
											uint32_t* puiLen,
											TickType_t xTimeout	)
{
	/*	Wait for  new data to be available, or timeout to be done	*/
	uint8_t ucIsNewDataAvailable = xSemaphoreTake(xRxAvailableSemaphore, xTimeout);

	/*	If no new data is available, return 0	*/
	if (!ucIsNewDataAvailable)
		return 0;

	/*	Otherwise, copy the RxBuffer to "pucBuffer"	*/
	for (uint32_t i = 0; i < *puiPortUsbRxBufferLen; i++)
		pucBuffer[i] = pucPortUsbRxBuffer[i];

	*puiLen = *puiPortUsbRxBufferLen;

	return 1;
}

















