/*
 * I2C.c
 *
 *  Created on: Jul 11, 2023
 *      Author: Ali Emad
 */

/*	LIB	*/
#include <stdint.h>

/*	FreeRTOS	*/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/*	MCAL (Ported)	*/
#include "MCAL_Port/Port_I2C.h"
#include "MCAL_Port/Port_Interrupt.h"

/*	SELF	*/
#include "HAL/I2C/I2C.h"

/*******************************************************************************
 * Helping types:
 ******************************************************************************/

/*******************************************************************************
 * Global variables:
 ******************************************************************************/

/*******************************************************************************
 * Helping functions / macros:
 * TODO: Waiting functions must have a timeout parameter.
 ******************************************************************************/
#define ADDRESS_HEADER_10_BIT		0b11110000

static uint8_t ucBlockUntilTxeOrNACK(uint8_t ucUnitNumber)
{
	uint8_t ucFlag;

	while(1)
	{
		if (ucPort_I2C_readTxEmptyFlag(ucUnitNumber))
		{
			vPort_I2C_clearTxEmptyFlag(ucUnitNumber);
			return 1;
		}

		ucFlag = ucPort_I2C_readAckErrFlag(ucUnitNumber);
		vPort_I2C_clearAckErrFlag(ucUnitNumber);
		if (ucFlag)
		{
			return 0;
		}
	}
}

static uint8_t ucBlockUntilAddressAckedOrNACK(uint8_t ucUnitNumber)
{
	uint8_t ucFlag;

	while(1)
	{
		if (ucPort_I2C_readAddressTxCompleteFlag(ucUnitNumber))
		{
			vPort_I2C_clearAddressTxCompleteFlag(ucUnitNumber);
			return 1;
		}

		ucFlag = ucPort_I2C_readAckErrFlag(ucUnitNumber);
		vPort_I2C_clearAckErrFlag(ucUnitNumber);
		if (ucFlag)
		{
			return 0;
		}
	}
}

/*
 * This function performs the following sequence:
 * 	-	Sends start (or repeated start) condition.
 * 	-	Sends address.
 *	-	Waits for address ACK.
 *
 * If function fails or address was not ACKed, it returns 0. Otherwise returns 1.
 */
static uint8_t ucStartConnection(xHOS_I2C_transreceiveParams_t* pxParams, uint8_t ucRW)
{
	uint8_t ucFlag;

	/*	Generate start condition	*/
	vPort_I2C_generateStart(pxParams->ucUnitNumber);

	/*	Block task until start bit is successfully sent	*/
	while(!ucPort_I2C_readStartConditionTxCompleteFlag(pxParams->ucUnitNumber));
	vPort_I2C_clearStartConditionTxCompleteFlag(pxParams->ucUnitNumber);

	/*	Send address	*/
	if (pxParams->ucIs7BitAddress)
		vPort_I2C_writeDrImm(pxParams->ucUnitNumber, (((uint8_t)pxParams->usAddress) << 1) | ucRW);

	else
	{
		/*	send header	*/
		vPort_I2C_writeDrImm(
			pxParams->ucUnitNumber,
			ADDRESS_HEADER_10_BIT | ((pxParams->usAddress >> 7) & 0b110));

		/*
		 * Block until TxBuffer/Register is empty, with reading ACK-failure flag,
		 * if ACK fails (received NACK) send stop condition and return.
		 */
		ucFlag = ucBlockUntilTxeOrNACK(pxParams->ucUnitNumber);
		if (ucFlag == 0)
		{
			vPort_I2C_generateStop(pxParams->ucUnitNumber);
			return 0;
		}

		/*	send 2nd address byte	*/
		vPort_I2C_writeDrImm(
			pxParams->ucUnitNumber,
			(pxParams->usAddress & 0xFF) | ucRW);
	}

	/*
	 * Block until address is ACKed, with reading ACK-failure flag,
	 * if ACK fails (received NACK) send stop condition and return.
	 */
	ucFlag = ucBlockUntilAddressAckedOrNACK(pxParams->ucUnitNumber);
	if (ucFlag == 0)
	{
		vPort_I2C_generateStop(pxParams->ucUnitNumber);
		return 0;
	}

	return 1;
}

/*******************************************************************************
 * API functions:
 ******************************************************************************/
/*
 * See header file for info.
 */
__attribute__((always_inline)) inline
void vHOS_I2C_enable(uint8_t ucUnitNumber)
{
	vPort_I2C_enable(ucUnitNumber);
}

/*
 * See header file for info.
 */
__attribute__((always_inline)) inline
void vHOS_I2C_disable(uint8_t ucUnitNumber)
{
	vPort_I2C_disable(ucUnitNumber);
}

/*
 * See header file for info.
 */
uint8_t ucHOS_I2C_masterTransReceive(xHOS_I2C_transreceiveParams_t* pxParams)
{
	uint8_t ucFlag;

	/*
	 * TODO: Driver should handle I2C transreceiption without entering such long
	 * time critical sections.
	 */
//	taskENTER_CRITICAL();

	/*	Wait for bus if it was busy	*/
	while(ucPort_I2C_readBusBusyFlag(pxParams->ucUnitNumber));
	vPort_I2C_clearBusBusyFlag(pxParams->ucUnitNumber);

	/*	Send start condition and address, with R/W bit reset (write)	*/
	ucFlag = ucStartConnection(pxParams, 0);
	if (ucFlag == 0)
	{
		vPort_I2C_generateStop(pxParams->ucUnitNumber);
//		taskEXIT_CRITICAL();
//		__BKPT(0);
		return 0;
	}

	/*	transmit Tx array	*/
	for (uint32_t i = 0; i < pxParams->uiTxSize; i++)
	{
		/*	write data	*/
		vPort_I2C_writeDrImm(pxParams->ucUnitNumber, pxParams->pucTxArr[i]);

		/*
		 * Block until TxBuffer/Register is empty, with reading ACK-failure flag,
		 * if ACK fails (received NACK) send stop condition and return.
		 */
		ucFlag = ucBlockUntilTxeOrNACK(pxParams->ucUnitNumber);
		if (ucFlag == 0)
		{
			vPort_I2C_generateStop(pxParams->ucUnitNumber);
//			taskEXIT_CRITICAL();
//			__BKPT(0);
			return 0;
		}
	}

	/*	if there's no data to be received, return	*/
	if (pxParams->uiRxSize == 0)
	{
		vPort_I2C_generateStop(pxParams->ucUnitNumber);
//		taskEXIT_CRITICAL();
		return 1;
	}

	/*	Send repeated start condition and address, with R/W bit set (read)	*/
	ucFlag = ucStartConnection(pxParams, 1);
	if (ucFlag == 0)
	{
		vPort_I2C_generateStop(pxParams->ucUnitNumber);
//		taskEXIT_CRITICAL();
//		__BKPT(0);
		return 0;
	}

	/*	Enable master-ACK on byte reception	*/
	vPort_I2C_enableAck(pxParams->ucUnitNumber);

	/*	Receive data until the before-last byte	*/
	for (uint32_t i = 0; i < pxParams->uiRxSize - 1; i++)
	{
		/*	Block until Rx Buffer/register is not empty	*/
		while(!ucPort_I2C_readRxNotEmptyFlag(pxParams->ucUnitNumber));
		vPort_I2C_clearRxNotEmptyFlag(pxParams->ucUnitNumber);

		/*	Read new received data to RxArr	*/
		pxParams->pucRxArr[i] = ucPort_I2C_readDrImm(pxParams->ucUnitNumber);
	}

	/*	Disable master-ack on byte reception	*/
	vPort_I2C_disableAck(pxParams->ucUnitNumber);

	/*	Generate stop condition.	*/
	vPort_I2C_generateStop(pxParams->ucUnitNumber);

	/*	Receive the last data byte	*/
	/*	Block until Rx Buffer/register is not empty	*/
	while(!ucPort_I2C_readRxNotEmptyFlag(pxParams->ucUnitNumber));
	vPort_I2C_clearRxNotEmptyFlag(pxParams->ucUnitNumber);

	/*	Read new received data to RxArr	*/
	pxParams->pucRxArr[pxParams->uiRxSize-1] = ucPort_I2C_readDrImm(pxParams->ucUnitNumber);

	/*	return successful	*/
//	taskEXIT_CRITICAL();
	return 1;
}









