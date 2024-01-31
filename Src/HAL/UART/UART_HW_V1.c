/*
 * UART_HW_V1.c
 *
 *  Created on: Sep 14, 2023
 *      Author: Ali Emad
 *
 * This is the UART source code used for targets which do not have DMA.
 */

/*	LIB	*/
#include <stdint.h>

/*	FreeRTOS	*/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/*	MCAL (Ported)	*/
#include "MCAL_Port/Port_UART.h"
#include "MCAL_Port/Port_Interrupt.h"
#include "MCAL_Port/Port_GPIO.h"
#include "MCAL_Port/Port_DIO.h"

/*	HAL (OS)	*/
#include "HAL/DMA/DMA.h"

/*	SELF	*/
#include "HAL/UART/UART_Config.h"

#if portDMA_IS_AVAILABLE == 0

/*******************************************************************************
 * Structures:
 ******************************************************************************/
typedef struct{
	/*	Transmission byte counter	*/
	uint32_t uiTxByteCounter;

	/*	Size of the current Tx data	*/
	uint32_t uiTxCurrentSize;
}xTxInfo_t;

typedef struct{
	/*	Pointer to the Rx buffer	*/
	int8_t* pcRxBuffer;

	/*	Size of the data received since calling "receive" function	*/
	uint32_t uiRxCurrentSize;

	/*	Total size of the data to be received	*/
	uint32_t uiRxTotalSize;
}xRxInfo_t;

typedef struct{
	/*
	 * Unit's mutex.
	 *
	 * (Taken by the task which is going to use the unit)
	 */
	StaticSemaphore_t xUnitMutexStatic;
	SemaphoreHandle_t xUnitMutex;

	/*
	 * Unit's Transfer complete binary semaphore (flag).
	 *
	 * (After task has took the "xUnitMutex", it can synchronize with end of
	 * transfer using this semaphore).
	 */
	StaticSemaphore_t xTransferCompleteSemaphoreStatic;
	SemaphoreHandle_t xTransferCompleteSemaphore;

	/*
	 * Unit's TxE (Tx buffer empty) binary semaphore (flag).
	 *
	 * After writing a byte to data register of the unit, byte is copied by
	 * HW to the final output shift register. Once this copy is done, unit raises
	 * the TxE flag.
	 *
	 * Within a transfer, if DMA is not used, driver would send a byte, and
	 * block until end of copy to the shift register. This blocking operation
	 * uses the following semaphore.
	 */
	StaticSemaphore_t xTxeSemaphoreStatic;
	SemaphoreHandle_t xTxeSemaphore;

	/*
	 * Unit's RxNE (Rx buffer not empty) binary semaphore (flag).
	 */
	StaticSemaphore_t xRxneSemaphoreStatic;
	SemaphoreHandle_t xRxneSemaphore;

	/*	Unit number	*/
	uint8_t ucUnitNumber;

	/*	Tx info	*/
	xTxInfo_t xTxInfo;

	/*	Rx info	*/
	xRxInfo_t xRxInfo;
}xHOS_UART_Unit_t;


/*******************************************************************************
 * Helping functions/macros:
 ******************************************************************************/
/*
 * Number of units to be created.
 */
#if uiCONF_UART_NUMBER_OF_NEEDED_UNITS > portUART_NUMBER_OF_UNITS
#define uiNUMBER_OF_UNITS			portUART_NUMBER_OF_UNITS
#else
#define uiNUMBER_OF_UNITS			uiCONF_UART_NUMBER_OF_NEEDED_UNITS
#endif

/*******************************************************************************
 * Global and static variables:
 ******************************************************************************/
/*
 * Array of units.
 */
static xHOS_UART_Unit_t pxUnitArr[uiNUMBER_OF_UNITS];


/*******************************************************************************
 * ISR callback:
 ******************************************************************************/
static void vTxeCallback(void* pvParams)
{
	xHOS_UART_Unit_t* pxUnit = (xHOS_UART_Unit_t*)pvParams;
	BaseType_t xHighPriorityTaskWoken = pdFALSE;

	vPort_UART_disableTxeInterrupt(pxUnit->ucUnitNumber);

	xSemaphoreGiveFromISR(	pxUnit->xTxeSemaphore,
							&xHighPriorityTaskWoken	);

	portYIELD_FROM_ISR(xHighPriorityTaskWoken);
}

static void vRxneCallback(void* pvParams)
{
	xHOS_UART_Unit_t* pxUnit = (xHOS_UART_Unit_t*)pvParams;

	pxUnit->xRxInfo.pcRxBuffer[pxUnit->xRxInfo.uiRxCurrentSize++] =
		ucPort_UART_readByte(pxUnit->ucUnitNumber);

	if (pxUnit->xRxInfo.uiRxCurrentSize == pxUnit->xRxInfo.uiRxTotalSize)
	{
		BaseType_t xHighPriorityTaskWoken = pdFALSE;

		xSemaphoreGiveFromISR(	pxUnit->xRxneSemaphore,
								&xHighPriorityTaskWoken	);

		portYIELD_FROM_ISR(xHighPriorityTaskWoken);
	}
}

static void vTcCallback(void* pvParams)
{
	xHOS_UART_Unit_t* pxUnit = (xHOS_UART_Unit_t*)pvParams;

	if (pxUnit->xTxInfo.uiTxByteCounter != pxUnit->xTxInfo.uiTxCurrentSize)
		return;

	BaseType_t xHighPriorityTaskWoken = pdFALSE;

	vPort_UART_disableTcInterrupt(pxUnit->ucUnitNumber);

	xSemaphoreGiveFromISR(	pxUnit->xTransferCompleteSemaphore,
							&xHighPriorityTaskWoken	);

	portYIELD_FROM_ISR(xHighPriorityTaskWoken);
}

/*******************************************************************************
 * Driver functions:
 ******************************************************************************/
/*
 * Initializes all HW based UART units.
 */
void vHOS_UART_HW_init(void)
{
	xHOS_UART_Unit_t* pxUnit;

	for (uint8_t i = 0; i < uiNUMBER_OF_UNITS; i++)
	{
		pxUnit = &pxUnitArr[i];

		/*	create unit's mutex	*/
		pxUnit->xUnitMutex =
			xSemaphoreCreateMutexStatic(&pxUnit->xUnitMutexStatic);
		xSemaphoreGive(pxUnit->xUnitMutex);

		/*	create transfer complete semaphore	*/
		pxUnit->xTransferCompleteSemaphore =
			xSemaphoreCreateBinaryStatic(&pxUnit->xTransferCompleteSemaphoreStatic);
		xSemaphoreGive(pxUnit->xTransferCompleteSemaphore);

		/*	create TxE semaphore	*/
		pxUnit->xTxeSemaphore =
			xSemaphoreCreateBinaryStatic(&pxUnit->xTxeSemaphoreStatic);
		xSemaphoreGive(pxUnit->xTxeSemaphore);

		/*	create RxNE semaphore	*/
		pxUnit->xRxneSemaphore =
			xSemaphoreCreateBinaryStatic(&pxUnit->xRxneSemaphoreStatic);
		xSemaphoreTake(pxUnit->xRxneSemaphore, 0);

		/*	Initialize unit's TxE interrupt	*/
		VPORT_INTERRUPT_SET_PRIORITY(	pxPortInterruptUartTxeIrqNumberArr[i],
										configLIBRARY_LOWEST_INTERRUPT_PRIORITY	);

		vPORT_INTERRUPT_ENABLE_IRQ(pxPortInterruptUartTxeIrqNumberArr[i]);

		vPort_UART_setTxeCallback(i, vTxeCallback, (void*)pxUnit);

		vPort_UART_disableTxeInterrupt(i);

		/*	Initialize unit's RxNE interrupt	*/
		VPORT_INTERRUPT_SET_PRIORITY(	pxPortInterruptUartRxneIrqNumberArr[i],
										configLIBRARY_LOWEST_INTERRUPT_PRIORITY	);

		vPORT_INTERRUPT_ENABLE_IRQ(pxPortInterruptUartRxneIrqNumberArr[i]);

		vPort_UART_setRxneCallback(i, vRxneCallback, (void*)pxUnit);

		vPort_UART_disableRxneInterrupt(i);

		/*	Initialize unit's TC interrupt	*/
		VPORT_INTERRUPT_SET_PRIORITY(	pxPortInterruptUartTcIrqNumberArr[i],
										configLIBRARY_LOWEST_INTERRUPT_PRIORITY	);

		vPORT_INTERRUPT_ENABLE_IRQ(pxPortInterruptUartTcIrqNumberArr[i]);

		vPort_UART_setTcCallback(i, vTcCallback, (void*)pxUnit);

		vPort_UART_disableTxeInterrupt(i);

		/*	Unit number	*/
		pxUnit->ucUnitNumber = i;

		/*	Enable unit	*/
		vPort_UART_enable(i);
	}
}

/*
 * Sends data using a HW unit.
 */
void vHOS_UART_HW_send(uint8_t ucUnitNumber, int8_t* pcArr, uint32_t uiSize)
{
	xHOS_UART_Unit_t* pxUnit = &pxUnitArr[ucUnitNumber];

	if (uiSize == 0)
	{
		xSemaphoreGive(pxUnit->xTransferCompleteSemaphore);
		return;
	}

	/*	Assure Transfer complete semaphore is not available (force it)	*/
	xSemaphoreTake(pxUnit->xTransferCompleteSemaphore, 0);

	/*	Assure TxE semaphore is not available (force it)	*/
	xSemaphoreTake(pxUnit->xTxeSemaphore, 0);

	pxUnit->xTxInfo.uiTxCurrentSize = uiSize;

	uint32_t* puiCnt = &pxUnit->xTxInfo.uiTxByteCounter;
	for (*puiCnt = 0; *puiCnt < uiSize; (*puiCnt)++)
	{
		/*	Write next byte to uint's data buffer (data register in most cases)	*/
		vPort_UART_sendByte(ucUnitNumber, pcArr[*puiCnt]);

		/*	Enable TxE interrupt (disabled in the ISR)	*/
		vPort_UART_enableTxeInterrupt(ucUnitNumber);

		/*	Block until there's free byte/s in unit's data buffer	*/
		xSemaphoreTake(pxUnit->xTxeSemaphore, portMAX_DELAY);
	}

	/*	Enable TC interrupt (disabled in the ISR)	*/
	vPort_UART_enableTcInterrupt(ucUnitNumber);
}

/*
 * Receives data using a HW unit.
 */
uint8_t ucHOS_UART_HW_receive(		uint8_t ucUnitNumber,
								int8_t* pcInArr,
								uint32_t uiSize,
								TickType_t xTimeout	)
{
	xHOS_UART_Unit_t* pxUnit = &pxUnitArr[ucUnitNumber];
	uint8_t ucState = 0;

//	/*	Flush	*/
//	(volatile void)ucPort_UART_readByte(ucUnitNumber);

	/*	Configure Rx info	*/
	pxUnit->xRxInfo.pcRxBuffer = pcInArr;
	pxUnit->xRxInfo.uiRxCurrentSize = 0;
	pxUnit->xRxInfo.uiRxTotalSize = uiSize;

	/*	Enable RxNE interrupt	*/
	vPort_UART_enableRxneInterrupt(ucUnitNumber);

	/*	Wait until data is received or timeout passes	*/
	if (xSemaphoreTake(pxUnit->xRxneSemaphore, xTimeout))
		ucState = 1;

	/*	Disable RxNE interrupt	*/
	vPort_UART_disableRxneInterrupt(ucUnitNumber);

	return ucState;
}

/*
 * Locks a HW unit.
 */
uint8_t ucHOS_UART_HW_takeMutex(uint8_t ucUnitNumber, TickType_t xTimeout)
{
	return xSemaphoreTake(pxUnitArr[ucUnitNumber].xUnitMutex, xTimeout);
}

/*
 * Unlocks a HW unit.
 */
void vHOS_UART_HW_releaseMutex(uint8_t ucUnitNumber)
{
	xSemaphoreGive(pxUnitArr[ucUnitNumber].xUnitMutex);
}

/*
 * Blocks untill end of current transmission.
 */
uint8_t ucHOS_UART_HW_blockUntilTransmissionComplete(uint8_t ucUnitNumber, TickType_t xTimeout)
{
	return xSemaphoreTake(pxUnitArr[ucUnitNumber].xTransferCompleteSemaphore, xTimeout);
}


#endif		/*		portDMA_IS_AVAILABLE == 0		*/

