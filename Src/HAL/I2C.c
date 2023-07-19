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
 ******************************************************************************/
#define ADDRESS_HEADER_10_BIT		0b11110000

static inline uint8_t ucBlockUntilTxeOrNACK(uint8_t ucUnitNumber)
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

/*******************************************************************************
 * API functions:
 ******************************************************************************/
/*
 * See header for info.
 */
void vHOS_I2C_init(uint8_t ucUnitNumber, xHOS_I2C_init_t* pxInitHandle)
{
	/*	Disable unit before initialization	*/
	vPort_I2C_disable(ucUnitNumber);

	/*	HW dependent initialization sequence	*/
	vPort_I2C_init(ucUnitNumber);

	/*	General call setting	*/
	if (pxInitHandle->ucEnableGeneralCall)
		vPort_I2C_enableGeneralCall(ucUnitNumber);
	else
		vPort_I2C_disableGeneralCall(ucUnitNumber);

	/*	Clock stretching setting	*/
	if (pxInitHandle->ucEnableClockStretching)
		vPort_I2C_enableClockStretching(ucUnitNumber);
	else
		vPort_I2C_disableClockStretching(ucUnitNumber);

	/*	Self address setting	*/
	vPort_I2C_writeOwnSlaveAddress(ucUnitNumber, pxInitHandle->usSelfAddress, pxInitHandle->ucIsAddress7Bit);

	/*	Clock mode and frequency setting	*/
	vPort_I2C_setClockModeAndFreq(ucUnitNumber, pxInitHandle->ucClockMode, pxInitHandle->uiSclFrequencyHz);

	/*	Max rising time setting	*/
	vPort_I2C_setMaxRisingTime(ucUnitNumber, pxInitHandle->uiMaxRisingTimeNs);
}

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
 * See header for info.
 */
uint8_t ucHOS_I2C_masterTransmit(	uint8_t ucUnitNumber,
									uint8_t* pucArr,
									uint32_t uiSize,
									uint16_t usAddress,
									uint8_t ucIs7BitAddress	)
{
	uint8_t ucFlag;

	taskENTER_CRITICAL();

	/*	Wait for bus if it was busy	*/
	while(ucPort_I2C_readBusBusyFlag(ucUnitNumber));
	vPort_I2C_clearBusBusyFlag(ucUnitNumber);

	/*	Generate start condition	*/
	vPort_I2C_generateStart(ucUnitNumber);

	/*	Block task until start bit is successfully sent	*/
	while(!ucPort_I2C_readStartConditionTxCompleteFlag(ucUnitNumber));
	vPort_I2C_clearStartConditionTxCompleteFlag(ucUnitNumber);

	/*	Send address	*/
	if (ucIs7BitAddress)
		while(1)
		vPort_I2C_writeDrImm(ucUnitNumber, ((uint8_t)usAddress) << 1);

	else
	{
		/*	send header	*/
		vPort_I2C_writeDrImm(
			ucUnitNumber,
			ADDRESS_HEADER_10_BIT | ((usAddress >> 7) & 0b110));

		/*
		 * Block until TxBuffer/Register is empty, with reading ACK-failure flag,
		 * if ACK fails (received NACK) send stop condition and return.
		 */
		if (ucBlockUntilTxeOrNACK(ucUnitNumber) == 0)
		{
			vPort_I2C_generateStop(ucUnitNumber);
			taskEXIT_CRITICAL();
			return 0;
		}

		/*	send 2nd address byte	*/
		vPort_I2C_writeDrImm(
			ucUnitNumber,
			usAddress & 0xFF);
	}

	/*
	 * Block until TxBuffer/Register is empty, with reading ACK-failure flag,
	 * if ACK fails (received NACK) send stop condition and return.
	 */
	if (ucBlockUntilTxeOrNACK(ucUnitNumber) == 0)
	{
		vPort_I2C_generateStop(ucUnitNumber);
		taskEXIT_CRITICAL();
		return 0;
	}

	/*	transmit byte array	*/
	for (uint32_t i = 0; i < uiSize; i++)
	{
		/*	write data	*/
		vPort_I2C_writeDrImm(ucUnitNumber, pucArr[i]);

		/*
		 * Block until TxBuffer/Register is empty, with reading ACK-failure flag,
		 * if ACK fails (received NACK) send stop condition and return.
		 */
		if (ucBlockUntilTxeOrNACK(ucUnitNumber) == 0)
		{
			vPort_I2C_generateStop(ucUnitNumber);
			taskEXIT_CRITICAL();
			return 0;
		}
	}

	/*	Send stop condition	*/
	vPort_I2C_generateStop(ucUnitNumber);

	/*	return successful	*/
	taskEXIT_CRITICAL();
	return 1;
}

/*
 * See header for info.
 */
uint8_t ucHOS_I2C_slaveReceive(	uint8_t ucUnitNumber,
								uint8_t* pucArr,
								uint32_t uiMaxSize,
								uint32_t* puiSize	)
{
	/*	Enable ACK transmitting on receiving a new byte	*/
	vPort_I2C_enableAck(ucUnitNumber);

	for (*puiSize = 0; (*puiSize) < uiMaxSize; (*puiSize)++)
	{
		/*	wait for receiving new byte or receiving a stop condition	*/
		while(1)
		{
			if (ucPort_I2C_readStopCondFlag(ucUnitNumber))
			{
				taskEXIT_CRITICAL();
				return 1;
			}

			if (ucPort_I2C_readRxNotEmptyFlag(ucUnitNumber))
				break;
		}

		/*	Read new byte	*/
		pucArr[*puiSize] = ucPort_I2C_readDrImm(ucUnitNumber);
	}

	taskEXIT_CRITICAL();
	return 0;
}











