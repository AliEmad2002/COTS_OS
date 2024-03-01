/*
 * SPI_V1.c
 *
 *  Created on: Jun 17, 2023
 *      Author: Ali Emad
 *
 * This is the SPI source code used for targets which do not have DMA.
 */

/*	LIB	*/
#include <stdint.h>

/*	FreeRTOS	*/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/*	MCAL (Ported)	*/
#include "MCAL_Port/Port_SPI.h"
#include "MCAL_Port/Port_Interrupt.h"
#include "MCAL_Port/Port_GPIO.h"

/*	HAL (OS)	*/
#include "HAL/DMA/DMA.h"

/*	SELF	*/
#include "HAL/SPI/SPI.h"

#if portDMA_IS_AVAILABLE == 0

/*******************************************************************************
 * Structures:
 ******************************************************************************/
typedef struct{
	/*
	 * Unit's mutex.
	 *
	 * (Taken by the task which is going to use the SPI unit)
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
	 * After writing a byte to data register of the SPI unit, byte is copied by
	 * HW to the final output shift register. Once this copy is done, unit raises
	 * the TxE flag.
	 *
	 * Within a transfer, if DMA is not used, SPI driver would send a byte, and
	 * block until end of copy to the shift register. This blocking operation
	 * uses the following semaphore.
	 */
	StaticSemaphore_t xTxeSemaphoreStatic;
	SemaphoreHandle_t xTxeSemaphore;

	/*
	 * Unit's RxNE (Rx buffer not empty) binary semaphore (flag).
	 *
	 * After writing a byte to data register of the SPI unit, byte is copied by
	 * HW to the final output shift register and then outputted on the bus. Once
	 * all 8-bits are outputted successfully, unit raises the RxNE flag.
	 *
	 * Within a transfer, if DMA is not used, SPI driver would send a byte, and
	 * block until last bit of the 8-bits is outputted on the bus. To assure end
	 * of transfer. This blocking operation uses the following semaphore.
	 */
	StaticSemaphore_t xRxneSemaphoreStatic;
	SemaphoreHandle_t xRxneSemaphore;

	/*	Unit's byte direction setting	*/
	uint8_t ucByteDir;

	/*	Unit number	*/
	uint8_t ucUnitNumber;
}xHOS_SPI_Unit_t;

/*******************************************************************************
 * Global and static variables:
 ******************************************************************************/
/*
 * Array of units.
 */
static xHOS_SPI_Unit_t pxUnitArr[portSPI_NUMBER_OF_UNITS];

/*******************************************************************************
 * Helping functions/macros:
 ******************************************************************************/
#define vCONF_ITERATOR(ucByteDir, uiSize, i, iIncrementer, iEnd)    \
{                                                                   \
	if ((ucByteDir) == ucHOS_SPI_BYTE_DIRECTION_MSBYTE_FIRST)       \
	{                                                               \
		(i) = (uiSize) - 1;                                         \
		(iIncrementer) = -1;                                        \
		(iEnd) = -1;                                                \
	}                                                               \
	else                                                            \
	{                                                               \
		(i) = 0;                                                    \
		(iIncrementer) = 1;                                         \
		(iEnd) = (uiSize);                                          \
	}                                                               \
}

static void vBlockUntilNotBusy(xHOS_SPI_Unit_t* pxUnit)
{
	/*
	 * Since busy flag is cleared right after RxNE rising edge of the last byte,
	 * block indefinitely on the RxNE semaphore, and after each RxNE event, check
	 * the busy flag.
	 */

	/*	Assure RxNE semaphore is not available (force it)	*/
	xSemaphoreTake(pxUnit->xRxneSemaphore, 0);

	while(1)
	{
		/*	Enable RxNE interrupt	*/
		vPORT_SPI_enableRxneInterrupt(pxUnit->ucUnitNumber);

		/*
		 * Block until the Rx buffer is not empty (i.e: a byte has completely been
		 * transmitted on the bus).
		 */
		xSemaphoreTake(pxUnit->xRxneSemaphore, portMAX_DELAY);

		vPORT_SPI_clearRxneFlag(pxUnit->ucUnitNumber);

		/*	Check busy flag, return if it is not set	*/
		if (!ucPort_SPI_isBusy(pxUnit->ucUnitNumber))
			return;
	}
}

/*******************************************************************************
 * ISR callback:
 ******************************************************************************/
static void vTxeCallback(void* pvParams)
{
	xHOS_SPI_Unit_t* pxUnit = (xHOS_SPI_Unit_t*)pvParams;
	BaseType_t xHighPriorityTaskWoken = pdFALSE;

	vPORT_SPI_disableTxeInterrupt(pxUnit->ucUnitNumber);

	xSemaphoreGiveFromISR(	pxUnit->xTxeSemaphore,
							&xHighPriorityTaskWoken	);

	portYIELD_FROM_ISR(xHighPriorityTaskWoken);
}

static void vRxneCallback(void* pvParams)
{
	xHOS_SPI_Unit_t* pxUnit = (xHOS_SPI_Unit_t*)pvParams;
	BaseType_t xHighPriorityTaskWoken = pdFALSE;

	vPORT_SPI_disableRxneInterrupt(pxUnit->ucUnitNumber);

	xSemaphoreGiveFromISR(	pxUnit->xRxneSemaphore,
							&xHighPriorityTaskWoken	);

	portYIELD_FROM_ISR(xHighPriorityTaskWoken);
}

/*******************************************************************************
 * API functions:
 ******************************************************************************/
/*
 * See header for info.
 */
void vHOS_SPI_init(void)
{
	xHOS_SPI_Unit_t* pxUnit;

	for (uint8_t i = 0; i < portSPI_NUMBER_OF_UNITS; i++)
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
		xSemaphoreGive(pxUnit->xRxneSemaphore);

		/*	Initialize unit's TxE interrupt	*/
		VPORT_INTERRUPT_SET_PRIORITY(	pxPortInterruptSpiTxeIrqNumberArr[i],
										configLIBRARY_LOWEST_INTERRUPT_PRIORITY	);

		vPORT_INTERRUPT_ENABLE_IRQ(pxPortInterruptSpiTxeIrqNumberArr[i]);

		vPort_SPI_setTxeCallback(i, vTxeCallback, (void*)pxUnit);

		vPORT_SPI_disableTxeInterrupt(i);

		/*	Initialize unit's RxNE interrupt	*/
		VPORT_INTERRUPT_SET_PRIORITY(	pxPortInterruptSpiRxneIrqNumberArr[i],
										configLIBRARY_LOWEST_INTERRUPT_PRIORITY	);

		vPORT_INTERRUPT_ENABLE_IRQ(pxPortInterruptSpiRxneIrqNumberArr[i]);

		vPort_SPI_setRxneCallback(i, vRxneCallback, (void*)pxUnit);

		vPORT_SPI_disableRxneInterrupt(i);

		/*	Byte direction is initially	"ucHOS_SPI_BYTE_DIRECTION_MSBYTE_FIRST"	*/
		pxUnit->ucByteDir = ucHOS_SPI_BYTE_DIRECTION_MSBYTE_FIRST;

		/*	Unit number	*/
		pxUnit->ucUnitNumber = i;
	}
}

/*
 * See header for info.
 */
__attribute__((always_inline)) inline
void inline vHOS_SPI_setByteDirection(uint8_t ucUnitNumber, uint8_t ucByteDirection)
{
	pxUnitArr[ucUnitNumber].ucByteDir = ucByteDirection;
}

/*
 * See header for info.
 */
void vHOS_SPI_send(uint8_t ucUnitNumber, int8_t* pcArr, uint32_t uiSize)
{
	xHOS_SPI_Unit_t* pxUnit = &pxUnitArr[ucUnitNumber];

	/*	Assure Transfer complete semaphore is not available (force it)	*/
	xSemaphoreTake(pxUnit->xTransferCompleteSemaphore, 0);

	/*	Assure TxE semaphore is not available (force it)	*/
	xSemaphoreTake(pxUnit->xTxeSemaphore, 0);

	/*	Assure that SPI unit is not busy	*/
	while(ucPort_SPI_isBusy(ucUnitNumber));

	/*	Configure loop counter based on the previously configured byte direction	*/
	int32_t i, iIncrementer, iEnd;
	vCONF_ITERATOR(pxUnit->ucByteDir, uiSize, i, iIncrementer, iEnd);

	for (; i != iEnd; i += iIncrementer)
	{
		/*	Write next byte to uint's data buffer (data register in most cases)	*/
		vPort_SPI_writeDataNoWait(ucUnitNumber, pcArr[i]);

		/*	Enable TxE interrupt (disabled in the ISR)	*/
		vPORT_SPI_enableTxeInterrupt(ucUnitNumber);

		/*	Block until there's free byte/s in unit's data buffer	*/
		xSemaphoreTake(pxUnit->xTxeSemaphore, portMAX_DELAY);

		/*	Assure that SPI unit is not busy	*/
		while(ucPort_SPI_isBusy(ucUnitNumber));
	}

	/*	Block until last byte is completely transmitted on the bus	*/
//	vBlockUntilNotBusy(pxUnit);

	/*	Acknowledge end of transfer	*/
	xSemaphoreGive(pxUnit->xTransferCompleteSemaphore);
}

/*
 * See header for info.
 */
void vHOS_SPI_transceive(	uint8_t ucUnitNumber,
							int8_t* pcOutArr,
							int8_t* pcInArr,
							uint32_t uiSize	)
{
	xHOS_SPI_Unit_t* pxUnit = &pxUnitArr[ucUnitNumber];

	/*	Assure Transfer complete semaphore is not available (force it)	*/
	xSemaphoreTake(pxUnit->xTransferCompleteSemaphore, 0);

	/*	Assure TxE semaphore is not available (force it)	*/
	xSemaphoreTake(pxUnit->xTxeSemaphore, 0);

	/*	Clear RxNE flag to avoid parasitic effect from previous transceive operation	*/
	vPORT_SPI_clearRxneFlag(ucUnitNumber);

	/*	Assure RxNE semaphore is not available (force it)	*/
	xSemaphoreTake(pxUnit->xRxneSemaphore, 0);

	/*	Configure loop counter based on the previously configured byte direction	*/
	int32_t i, iIncrementer, iEnd;
	vCONF_ITERATOR(pxUnit->ucByteDir, uiSize, i, iIncrementer, iEnd);

	int32_t iReader = i;

	for (; i != iEnd; i += iIncrementer)
	{
		/*	Write next byte to uint's data buffer (data register in most cases)	*/
		vPort_SPI_writeDataNoWait(ucUnitNumber, pcOutArr[i]);

		/*	Enable TxE & RxNE interrupts (disabled in the ISR)	*/
		vPORT_SPI_enableTxeInterrupt(ucUnitNumber);
		vPORT_SPI_enableRxneInterrupt(ucUnitNumber);

		/*	Block until there's free byte/s in unit's Tx buffer	*/
		xSemaphoreTake(pxUnit->xTxeSemaphore, portMAX_DELAY);

		/*	Block until there's new byte/s in unit's Rx buffer	*/
		xSemaphoreTake(pxUnit->xRxneSemaphore, portMAX_DELAY);

		/*	If there's an unread byte in the Rx buffer, read it	*/
		if (xSemaphoreTake(pxUnit->xRxneSemaphore, 0))
		{
			pcInArr[iReader] = ucPort_SPI_readDataNoWait(ucUnitNumber);
			iReader += iIncrementer;
		}
	}

	/*	Empty the Rx buffer (Keep reading into "pcInArr[]").	*/
	for (; iReader != iEnd; iReader += iIncrementer)
	{
		/*	Enable RxNE interrupt	*/
		vPORT_SPI_enableRxneInterrupt(pxUnit->ucUnitNumber);

		/*
		 * Block until the Rx buffer is not empty (i.e: a byte has completely been
		 * transmitted on the bus).
		 */
		xSemaphoreTake(pxUnit->xRxneSemaphore, 0);

		pcInArr[iReader] = ucPort_SPI_readDataNoWait(ucUnitNumber);
	}

	/*	Acknowledge end of transfer	*/
	xSemaphoreGive(pxUnit->xTransferCompleteSemaphore);
}

/*
 * See header for info.
 */
void vHOS_SPI_receive(		uint8_t ucUnitNumber,
							int8_t* pcInArr,
							uint32_t uiSize	)
{
	xHOS_SPI_Unit_t* pxUnit = &pxUnitArr[ucUnitNumber];

	/*	Assure Transfer complete semaphore is not available (force it)	*/
	xSemaphoreTake(pxUnit->xTransferCompleteSemaphore, 0);

	/*	Assure TxE semaphore is not available (force it)	*/
	xSemaphoreTake(pxUnit->xTxeSemaphore, 0);

	/*	Clear RxNE flag to avoid parasitic effect from previous transceive operation	*/
	vPORT_SPI_clearRxneFlag(ucUnitNumber);

	/*	Assure RxNE semaphore is not available (force it)	*/
	xSemaphoreTake(pxUnit->xRxneSemaphore, 0);

	/*	Configure loop counter based on the previously configured byte direction	*/
	int32_t i, iIncrementer, iEnd;
	vCONF_ITERATOR(pxUnit->ucByteDir, uiSize, i, iIncrementer, iEnd);

	int32_t iReader = i;

	for (; i != iEnd; i += iIncrementer)
	{
		/*	Write foo byte to uint's data buffer (data register in most cases)	*/
		vPort_SPI_writeDataNoWait(ucUnitNumber, 0xFF);

		/*	Enable TxE & RxNE interrupts (disabled in the ISR)	*/
		vPORT_SPI_enableTxeInterrupt(ucUnitNumber);
		vPORT_SPI_enableRxneInterrupt(ucUnitNumber);

		/*	Block until there's free byte/s in unit's Tx buffer	*/
		xSemaphoreTake(pxUnit->xTxeSemaphore, portMAX_DELAY);

		/*	Block until there's new byte/s in unit's Rx buffer	*/
		xSemaphoreTake(pxUnit->xRxneSemaphore, portMAX_DELAY);

		pcInArr[iReader] = ucPort_SPI_readDataNoWait(ucUnitNumber);
		iReader += iIncrementer;
	}

	/*	Acknowledge end of transfer	*/
	xSemaphoreGive(pxUnit->xTransferCompleteSemaphore);
}

/*
 * See header for info.
 */
void vHOS_SPI_sendMultiple(	uint8_t ucUnitNumber,
							int8_t* pcArr,
							uint32_t uiSize,
							uint32_t uiN	)
{
	xHOS_SPI_Unit_t* pxUnit = &pxUnitArr[ucUnitNumber];

	/*	Assure Transfer complete semaphore is not available (force it)	*/
	xSemaphoreTake(pxUnit->xTransferCompleteSemaphore, 0);

	/*	Assure TxE semaphore is not available (force it)	*/
	xSemaphoreTake(pxUnit->xTxeSemaphore, 0);

	/*	Assure that SPI unit is not busy	*/
	while(ucPort_SPI_isBusy(ucUnitNumber));

	/*	Configure loop counter based on the previously configured byte direction	*/
	int32_t i, iInit, iIncrementer, iEnd;
	vCONF_ITERATOR(pxUnit->ucByteDir, uiSize, iInit, iIncrementer, iEnd);

	while(uiN--)
	{
		i = iInit;

		for (; i != iEnd; i += iIncrementer)
		{
			/*	Write next byte to uint's data buffer (data register in most cases)	*/
			vPort_SPI_writeDataNoWait(ucUnitNumber, pcArr[i]);

			/*	Enable TxE interrupt (disabled in the ISR)	*/
			vPORT_SPI_enableTxeInterrupt(ucUnitNumber);

			/*	Block until there's free byte/s in unit's data buffer	*/
			xSemaphoreTake(pxUnit->xTxeSemaphore, portMAX_DELAY);

			/*	Assure that SPI unit is not busy	*/
			while(ucPort_SPI_isBusy(ucUnitNumber));
		}
	}

	/*	Block until last byte is completely transmitted on the bus	*/
	vBlockUntilNotBusy(pxUnit);

	/*	Acknowledge end of transfer	*/
	xSemaphoreGive(pxUnit->xTransferCompleteSemaphore);
}

/*
 * See header for info.
 */
inline __attribute__((always_inline))
uint8_t inline ucHOS_SPI_takeMutex(uint8_t ucUnitNumber, TickType_t xTimeout)
{
	return 1;//xSemaphoreTake(pxUnitArr[ucUnitNumber].xUnitMutex, xTimeout);
}

/*
 * See header for info.
 */
inline __attribute__((always_inline))
void inline vHOS_SPI_releaseMutex(uint8_t ucUnitNumber)
{
	xSemaphoreGive(pxUnitArr[ucUnitNumber].xUnitMutex);
}

/*
 * See header for info.
 */
inline __attribute__((always_inline))
uint8_t inline ucHOS_SPI_blockUntilTransferComplete(uint8_t ucUnitNumber, TickType_t xTimeout)
{
	return xSemaphoreTake(pxUnitArr[ucUnitNumber].xTransferCompleteSemaphore, xTimeout);
}













#endif		/*		portDMA_IS_AVAILABLE		*/





