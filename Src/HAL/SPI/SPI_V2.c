/*
 * SPI_V1.c
 *
 *  Created on: Jun 17, 2023
 *      Author: Ali Emad
 *
 * This is the SPI source code used for targets which have DMA.
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

/*	HAL (OS)	*/
#include "HAL/DMA/DMA.h"

/*	SELF	*/
#include "HAL/SPI/SPI.h"

#if portDMA_IS_AVAILABLE == 1

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

/*
 * Sends without DMA.
 *
 * (Used in case of byte direction configured in the SPI driver is different from
 * that which the DMA uses).
 */
static void vSendLoop(xHOS_SPI_Unit_t* pxUnit, int8_t* pcArr, uint32_t uiSize)
{
	/*	Configure loop counter based on the previously configured byte direction	*/
	int32_t i, iIncrementer, iEnd;
	vCONF_ITERATOR(pxUnit->ucByteDir, uiSize, i, iIncrementer, iEnd);

	for (; i != iEnd; i += iIncrementer)
	{
		/*	Write next byte to uint's data buffer (data register in most cases)	*/
		vPort_SPI_writeDataNoWait(pxUnit->ucUnitNumber, pcArr[i]);

		/*	Enable TxE interrupt (disabled in the ISR)	*/
		vPORT_SPI_enableTxeInterrupt(pxUnit->ucUnitNumber);

		/*	Block until there's free byte/s in unit's data buffer	*/
		xSemaphoreTake(pxUnit->xTxeSemaphore, portMAX_DELAY);
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

	/*
	 * If the configured byte direction is not same as DMA endianess, send bytes
	 * using the CPU.
	 */
	if (!(portDMA_ENDIANESS ^ pxUnit->ucByteDir))
	{
		/*	Send	*/
		vSendLoop(pxUnit, pcArr, uiSize);

		/*	Block until last byte is completely transmitted on the bus	*/
		vBlockUntilNotBusy(pxUnit);

		/*	Acknowledge end of transfer	*/
		xSemaphoreGive(pxUnit->xTransferCompleteSemaphore);

		return;
	}

	/*	Configure DMA transfer	*/
	xHOS_DMA_TransInfo_t xDmaInfo = {
		.pvMemoryStartingAdderss = (void*)pcArr,

		.pvPeripheralStartingAdderss = pvPort_SPI_getDrAddress(ucUnitNumber),

		.uiN = uiSize,

		.ucTriggerSource = 0,

		/*
		 * Communication transfers are given lowest priority as they consume much
		 * larger time than other transfers (like memory to memory transfer).
		 */
		.ucPriLevel = 0,

		.ucDirection = 1,

		.ucMemoryIncrement = 1,

		.ucPeripheralIncrement = 0
	};

	/*	Lock DMA channel	*/
	if (portSPI_IS_DMA_STATIC_CONNECTED)
	{
		xDmaInfo.ucUnitNumber = ppucPortSpiTxeDmaMapping[ucUnitNumber][0];
		xDmaInfo.ucChannelNumber = ppucPortSpiTxeDmaMapping[ucUnitNumber][1];

		ucHOS_DMA_lockChannel(	xDmaInfo.ucUnitNumber,
								xDmaInfo.ucChannelNumber,
								portMAX_DELAY	);
	}
	else
	{
		ucHOS_DMA_lockAnyChannel(	&xDmaInfo.ucUnitNumber,
									&xDmaInfo.ucChannelNumber,
									portMAX_DELAY	);
	}

	/*	Connect SPI unit to the locked DMA channel	*/
	vPort_SPI_connectTxeToDma(	ucUnitNumber,
								xDmaInfo.ucUnitNumber,
								xDmaInfo.ucChannelNumber	);

	/*	Start data transfer	*/
	vHOS_DMA_startTransfer(&xDmaInfo);

	/*	Block until end of transfer	*/
	ucHOS_DMA_blockUntilTransferComplete(	xDmaInfo.ucUnitNumber,
											xDmaInfo.ucChannelNumber,
											portMAX_DELAY	);

	/*	Enable TxE interrupt (disabled in the ISR)	*/
	vPORT_SPI_enableTxeInterrupt(pxUnit->ucUnitNumber);

	/*	Acquire TxE semaphore to assure last byte is transferred to the Tx buffer	*/
	xSemaphoreTake(pxUnit->xTxeSemaphore, portMAX_DELAY);

	/*
	 * Block until last byte is completely transmitted on the bus.
	 * (Check busy flag)
	 */
	while(ucPort_SPI_isBusy(pxUnit->ucUnitNumber));

	/*	Disconnect SPI unit from the locked DMA channel	*/
	vPort_SPI_disconnectTxeFromDma(ucUnitNumber);

	/*	Release DMA channel	*/
	ucHOS_DMA_releaseChannel(	xDmaInfo.ucUnitNumber,
								xDmaInfo.ucChannelNumber,
								portMAX_DELAY	);

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

		/*	Block until there's free byte/s in unit's data buffer	*/
		xSemaphoreTake(pxUnit->xTxeSemaphore, portMAX_DELAY);

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
		xSemaphoreTake(pxUnit->xRxneSemaphore, portMAX_DELAY);

		pcInArr[iReader] = ucPort_SPI_readDataNoWait(ucUnitNumber);
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

	/*
	 * If the configured byte direction is not same as DMA endianess, send bytes
	 * using the CPU.
	 */
	if (!(portDMA_ENDIANESS ^ pxUnit->ucByteDir))
	{
		/*	Send	*/
		while(uiN--)
			vSendLoop(pxUnit, pcArr, uiSize);

		/*	Block until last byte is completely transmitted on the bus	*/
		vBlockUntilNotBusy(pxUnit);

		/*	Acknowledge end of transfer	*/
		xSemaphoreGive(pxUnit->xTransferCompleteSemaphore);

		return;
	}

	/*	Configure DMA transfer	*/
	xHOS_DMA_TransInfo_t xDmaInfo = {
		.pvMemoryStartingAdderss = (void*)pcArr,

		.pvPeripheralStartingAdderss = pvPort_SPI_getDrAddress(ucUnitNumber),

		.uiN = uiSize,

		.ucTriggerSource = 0,

		/*
		 * Communication transfers are given lowest priority as they consume much
		 * larger time than other transfers (like memory to memory transfer).
		 */
		.ucPriLevel = 0,

		.ucDirection = 1,

		.ucMemoryIncrement = 1,

		.ucPeripheralIncrement = 0
	};

	/*	Lock DMA channel	*/
	if (portSPI_IS_DMA_STATIC_CONNECTED)
	{
		xDmaInfo.ucUnitNumber = ppucPortSpiTxeDmaMapping[ucUnitNumber][0];
		xDmaInfo.ucChannelNumber = ppucPortSpiTxeDmaMapping[ucUnitNumber][1];

		ucHOS_DMA_lockChannel(	xDmaInfo.ucUnitNumber,
								xDmaInfo.ucChannelNumber,
								portMAX_DELAY	);
	}
	else
	{
		ucHOS_DMA_lockAnyChannel(	&xDmaInfo.ucUnitNumber,
									&xDmaInfo.ucChannelNumber,
									portMAX_DELAY	);
	}

	/*	Connect SPI unit to the locked DMA channel	*/
	vPort_SPI_connectTxeToDma(	ucUnitNumber,
								xDmaInfo.ucUnitNumber,
								xDmaInfo.ucChannelNumber	);

	while(uiN--)
	{
		/*	Start data transfer	*/
		vHOS_DMA_startTransfer(&xDmaInfo);

		/*	Block until end of transfer	*/
		ucHOS_DMA_blockUntilTransferComplete(	xDmaInfo.ucUnitNumber,
												xDmaInfo.ucChannelNumber,
												portMAX_DELAY	);

		/*	Enable TxE interrupt (disabled in the ISR)	*/
		vPORT_SPI_enableTxeInterrupt(pxUnit->ucUnitNumber);

		/*	Acquire TxE semaphore to assure last byte is transferred to the Tx buffer	*/
		xSemaphoreTake(pxUnit->xTxeSemaphore, portMAX_DELAY);
	}

	/*
	 * Block until last byte is completely transmitted on the bus.
	 * (Check busy flag)
	 */
	while(ucPort_SPI_isBusy(pxUnit->ucUnitNumber));

	/*	Disconnect SPI unit from the locked DMA channel	*/
	vPort_SPI_disconnectTxeFromDma(ucUnitNumber);

	/*	Release DMA channel	*/
	ucHOS_DMA_releaseChannel(	xDmaInfo.ucUnitNumber,
								xDmaInfo.ucChannelNumber,
								portMAX_DELAY	);

	/*	Acknowledge end of transfer	*/
	xSemaphoreGive(pxUnit->xTransferCompleteSemaphore);
}

/*
 * See header for info.
 */
inline __attribute__((always_inline))
uint8_t inline ucHOS_SPI_takeMutex(uint8_t ucUnitNumber, TickType_t xTimeout)
{
	return xSemaphoreTake(pxUnitArr[ucUnitNumber].xUnitMutex, xTimeout);
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





