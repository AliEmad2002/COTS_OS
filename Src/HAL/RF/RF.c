/*
 * RF.c
 *
 *  Created on: Jul 26, 2023
 *      Author: Ali Emad
 */


/*	LIB	*/
#include "stdint.h"
#include "stdio.h"

/*	MCAL (ported)	*/
#include "MCAL_Port/Port_Breakpoint.h"
#include "MCAL_Port/Port_DIO.h"

/*	OS	*/
#include "FreeRTOS.h"
#include "task.h"
#include "RTOS_PRI_Config.h"

/*	HAL	*/

/*	SELF	*/
#include "HAL/RF/RF_private.h"
#include "HAL/RF/RF.h"


/*******************************************************************************
 * Task function:
 ******************************************************************************/
static void vRxTask(void* pvParams)
{
	xHOS_RF_t* pxHandle = (xHOS_RF_t*)pvParams;

	/*	Create pointer to the RxFrame stored in RxShiftRegister	*/
	xHOS_RF_Frame_t* pxRxFrame = (xHOS_RF_Frame_t*)(pxHandle->pucRxShiftRegister);

	/*	Task is initially suspended	*/
	vTaskSuspend(pxHandle->xRxTask);

	while(1)
	{
		/*	Block until physical layer is done receiving a complete frame	*/
		xSemaphoreTake(pxHandle->xPhySemaphore, portMAX_DELAY);

		/*
		 * If a new frame was received before the RxComplete flag was cleared,
		 * overrun flag is raised. And receiving tasks are suspended until user
		 * re-enables them using "vHOS_RF_enable()".
		 */
		if (pxHandle->ucRxCompleteFalg == 1)
		{
			pxHandle->ucOverrunFlag = 1;
			vTaskSuspend(pxHandle->xRxTask);
			continue;
		}

		/*
		 * Compare destination address of the received frame and handle's self
		 * address, if they don't match this frame is not meant for this handle.
		 */
		if (pxRxFrame->ucDestAddress != pxHandle->ucSelfAddress)
		{
			vTaskResume(pxHandle->xRxPhyTask);
			continue;
		}

		/*	Update SrcAddress filed.	*/
		pxHandle->ucSrcAddress = pxRxFrame->ucSrcAddress;

		/*
		 * If the received frame is an ACK frame, raise the ACK flag and block
		 * until the next frame is received.
		 */
		if (pxRxFrame->ucIsAck == 1)
		{
			pxHandle->ucAckFlag = 1;
			xSemaphoreGive(pxHandle->xAckSemaphore);
			vTaskResume(pxHandle->xRxPhyTask);
			continue;
		}

		/*
		 * Otherwise, update the receiver data buffer and CRC with a copy of
		 * RxShiftRegister's. And raise the RxComplete flag.
		 */
		for (uint32_t i = 0; i < uiRF_DATA_BYTES_PER_FRAME; i++)
		{
			pxHandle->pucRxBuffer[i] = pxRxFrame->pucData[i];
		}

		pxHandle->usRxCRC = (uint16_t)pxRxFrame->ucCRC0 | (((uint16_t)pxRxFrame->ucCRC1) << 8);

		pxHandle->ucRxCompleteFalg = 1;
		xSemaphoreGive(pxHandle->xRxCompleteSemaphore);

		/*	Resume physical layer's task	*/
		vTaskResume(pxHandle->xRxPhyTask);
	}
}


/*******************************************************************************
 * Physical layer functions:
 ******************************************************************************/
extern void xHOS_RFPhysical_init(xHOS_RF_t* pxHandle);
extern void xHOS_RFPhysical_enable(xHOS_RF_t* pxHandle);
extern void xHOS_RFPhysical_disable(xHOS_RF_t* pxHandle);
extern void xHOS_RFPhysical_startTransmission(xHOS_RF_t* pxHandle);

/*******************************************************************************
 * API functions:
 ******************************************************************************/
/*	See header for info	*/
void vHOS_RF_init(xHOS_RF_t* pxHandle)
{
	/*	Initialize physical layer	*/
	xHOS_RFPhysical_init(pxHandle);

	/*	Initialize data-link layer's flags	*/
	pxHandle->ucTxEmptyFalg = 1;	// Initially empty.
	pxHandle->ucRxCompleteFalg = 0;	// Initially no received frames yet.
	pxHandle->ucOverrunFlag = 0;	// Initially no overruns occurred.
	pxHandle->ucAckFlag = 0;		// Initially no ACKs were received.

	/*	Create data-link layer's semaphores	*/
	pxHandle->xTxEmptySemaphore = xSemaphoreCreateBinaryStatic(&pxHandle->xTxEmptySemaphoreStatic);
	xSemaphoreGive(pxHandle->xTxEmptySemaphore);	// Initially empty.

	pxHandle->xRxCompleteSemaphore = xSemaphoreCreateBinaryStatic(&pxHandle->xRxCompleteSemaphoreStatic);
	xSemaphoreTake(pxHandle->xRxCompleteSemaphore, 0);	// Initially no received frames yet.

	pxHandle->xAckSemaphore = xSemaphoreCreateBinaryStatic(&pxHandle->xAckSemaphoreStatic);
	xSemaphoreTake(pxHandle->xAckSemaphore, 0);	// Initially no ACKs were received.

	/*	Initialize link layer's task	*/
	static uint8_t ucCreatedObjectsCount = 0;
	char pcRxTaskName[configMAX_TASK_NAME_LEN];
	sprintf(pcRxTaskName, "RF_Rx%d", ucCreatedObjectsCount);

	pxHandle->xRxTask = xTaskCreateStatic(	vRxTask,
										pcRxTaskName,
										configMINIMAL_STACK_SIZE,
										(void*)pxHandle,
										configHOS_SOFT_REAL_TIME_TASK_PRI,
										pxHandle->puxRxTaskStack,
										&pxHandle->xRxTaskStatic	);
}

/*	See header for info	*/
void vHOS_RF_enable(xHOS_RF_t* pxHandle)
{
	/*	Enable physical layer	*/
	xHOS_RFPhysical_enable(pxHandle);

	/*	Resume link layer receiver's task	*/
	vTaskResume(pxHandle->xRxTask);
}

/*	See header for info	*/
void vHOS_RF_disable(xHOS_RF_t* pxHandle)
{
	/*	Disable physical layer	*/
	xHOS_RFPhysical_disable(pxHandle);

	/*	Suspend link layer receiver's task	*/
	vTaskSuspend(pxHandle->xRxTask);
}

/*	See header for info	*/
void vHOS_RF_send(	xHOS_RF_t* pxHandle,
					uint8_t ucDestAddress,
					uint8_t* pucData,
					uint32_t uiDataSizeInBytes,
					uint16_t usCRC	)
{
	/*
	 * Check if TxBuffer is empty. If not, raise the overrun flag and return.
	 */
	if (xSemaphoreTake(pxHandle->xTxEmptySemaphore, 0))
	{
		pxHandle->ucTxEmptyFalg = 0;
	}
	else
	{
		pxHandle->ucOverrunFlag = 1;
		return;
	}

	/*	Create pointer to the TxFrame stored in TxShiftRegister	*/
	xHOS_RF_Frame_t* pxTxFrame = (xHOS_RF_Frame_t*)(pxHandle->pucTxShiftRegister);

	/*	Write SOF and EOF to their constant values	*/
	pxTxFrame->ucSOF = ucRF_SOF;
	pxTxFrame->ucEOF = ucRF_EOF;

	/*	Write data to the TxShiftRegister, with the reset of data field cleared	*/
	uint32_t i = 0;

	for (; i < uiDataSizeInBytes; i++)
		pxTxFrame->pucData[i] = pucData[i];

	for (; i < uiRF_DATA_BYTES_PER_FRAME; i++)
			pxTxFrame->pucData[i] = 0;

	/*	Write CRC to the TxShiftRegister	*/
	pxTxFrame->ucCRC0 = usCRC & 0xFF;
	pxTxFrame->ucCRC1 = usCRC >> 8;

	/*	Write SrcAddress and DestAddress to the TxShiftRegister	*/
	pxTxFrame->ucSrcAddress = pxHandle->ucSelfAddress;
	pxTxFrame->ucDestAddress = ucDestAddress;

	/*	This  is not an ACK frame	*/
	pxTxFrame->ucIsAck = 0;

	/*	Start the transmission in physical layer	*/
	xHOS_RFPhysical_startTransmission(pxHandle);
}

/*	See header for info	*/
void vHOS_RF_sendAck(xHOS_RF_t* pxHandle, uint8_t ucDestAddress)
{
	/*
	 * Check if TxBuffer is empty. If not, raise the overrun flag and return.
	 */
	if (xSemaphoreTake(pxHandle->xTxEmptySemaphore, 0))
	{
		pxHandle->ucTxEmptyFalg = 0;
	}
	else
	{
		pxHandle->ucOverrunFlag = 1;
		return;
	}

	/*	Create pointer to the TxFrame stored in TxShiftRegister	*/
	xHOS_RF_Frame_t* pxTxFrame = (xHOS_RF_Frame_t*)(pxHandle->pucTxShiftRegister);

	/*	Write SOF and EOF to their constant values	*/
	pxTxFrame->ucSOF = ucRF_SOF;
	pxTxFrame->ucEOF = ucRF_EOF;

	/*	Write SrcAddress and DestAddress to the TxShiftRegister	*/
	pxTxFrame->ucSrcAddress = pxHandle->ucSelfAddress;
	pxTxFrame->ucDestAddress = ucDestAddress;

	/*	This  is an ACK frame	*/
	pxTxFrame->ucIsAck = 1;

	/*	Start the transmission in physical layer	*/
	xHOS_RFPhysical_startTransmission(pxHandle);
}

/*	See header for info	*/
__attribute__((always_inline)) inline
void vHOS_RF_blockUntilTxEmpty(xHOS_RF_t* pxHandle)
{
	xSemaphoreTake(pxHandle->xTxEmptySemaphore, portMAX_DELAY);
	xSemaphoreGive(pxHandle->xTxEmptySemaphore);
}

/*	See header for info	*/
__attribute__((always_inline)) inline
BaseType_t xHOS_RF_blockUntilRxComplete(xHOS_RF_t* pxHandle, TickType_t xTimeoutTicks)
{
	/*	Try to take semaphore	*/
	BaseType_t xFlag = xSemaphoreTake(pxHandle->xRxCompleteSemaphore, xTimeoutTicks);

	/*
	 * If it was successfully taken, give it again, as it must not be cleared
	 * within this function.
	 */
	if (xFlag)
		xSemaphoreGive(pxHandle->xRxCompleteSemaphore);

	return xFlag;
}

/*	See header for info	*/
__attribute__((always_inline)) inline
BaseType_t xHOS_RF_blockUntilAckReceived(xHOS_RF_t* pxHandle, TickType_t xTimeoutTicks)
{
	/*	Try to take semaphore	*/
	BaseType_t xFlag = xSemaphoreTake(pxHandle->xAckSemaphore, xTimeoutTicks);

	/*
	 * If it was successfully taken, give it again, as it must not be cleared
	 * within this function.
	 */
	if (xFlag)
		xSemaphoreGive(pxHandle->xAckSemaphore);

	return xFlag;
}

/*	See header for info	*/
__attribute__((always_inline)) inline
void vHOS_RF_clearRxComplete(xHOS_RF_t* pxHandle)
{
	pxHandle->ucRxCompleteFalg = 0;
}

/*	See header for info	*/
__attribute__((always_inline)) inline
void vHOS_RF_clearAck(xHOS_RF_t* pxHandle)
{
	pxHandle->ucAckFlag = 0;
	xSemaphoreTake(pxHandle->xAckSemaphore, 0);
}
















