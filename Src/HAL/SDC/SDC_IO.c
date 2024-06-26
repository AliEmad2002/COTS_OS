/*
 * SDC_IO.c
 *
 *  Created on: Aug 17, 2023
 *      Author: Ali Emad
 */

/*	LIB	*/
#include "stdint.h"
#include "LIB/CRC/CRC.h"

/*	RTOS	*/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/*	HAL	*/
#include "HAL/SPI/SPI.h"
#include "HAL/SDC/SDC.h"
#include "HAL/SDC/SDC_Private.h"
#include "HAL/SDC/SDC_CMD.h"
#include "HAL/SDC/SDC_init.h"

/*	SELF	*/
#include "HAL/SDC/SDC_IO.h"


uint8_t ucHOS_SDC_writeBlock(	xHOS_SDC_t* pxSdc,
								xHOS_SDC_Block_Buffer_t* pxBlock,
								TickType_t xTimeout	)
{
	SDC_R1_t xR1;
	SDC_Data_Response_t xRd;

	uint8_t ucSuccessful;
	uint8_t ucGotR1;
	uint8_t ucGotRd;
	uint8_t ucDummyByte = 0xFF;
	uint32_t uiAddress;

	/*	Acquire SPI mutex	*/
//	ucSuccessful = ucHOS_SPI_takeMutex(pxSdc->ucSpiUnitNumber, xTimeout);
//	if (!ucSuccessful)
//		return 0;

	/*	Get address (Based on SDC's version)	*/
	if (pxSdc->xVer == xHOS_SDC_Version_2_BlockAddress)
		uiAddress = pxBlock->uiLbaRead;
	else if (pxSdc->xVer == xHOS_SDC_Version_2_ByteAddress)
		uiAddress = pxBlock->uiLbaRead * 512;
	else
		uiAddress = 0;	/*	TODO: what does other versions (1, 3) do?	*/

	/*	Send CMD24	*/
	vHOS_SDC_sendCommand(pxSdc, 24, uiAddress);

	/*	Get R1 response	*/
	ucGotR1 = ucHOS_SDC_getR1(pxSdc, &xR1);
	if (ucGotR1 == 0 || ucHOS_SDC_checkR1(&xR1) == 0)
	{
//		vHOS_SPI_releaseMutex(pxSdc->ucSpiUnitNumber);
		return 0;
	}

	/*	if CRC was enabled, calculate it for the block	*/
	uint16_t usCrc = 0;
	if (pxSdc->ucIsCrcEnabled == 1)
		usCrc = usLIB_CRC_getCrc16(pxBlock->pucBufferr, 512);

	/*	wait for 1 SPI byte	*/
	vHOS_SPI_send(pxSdc->ucSpiUnitNumber, (int8_t*)&ucDummyByte, 1);

	/**	send the data packet	**/
	/*	send data token	*/
	ucDummyByte = 0b11111110;
	vHOS_SPI_send(pxSdc->ucSpiUnitNumber, (int8_t*)&ucDummyByte, 1);

	/*	send data block	*/
	vHOS_SPI_setByteDirection(	pxSdc->ucSpiUnitNumber,
								ucHOS_SPI_BYTE_DIRECTION_LSBYTE_FIRST	);
	vHOS_SPI_send(pxSdc->ucSpiUnitNumber, (int8_t*)pxBlock->pucBufferr, 512);

	/*	send CRC	*/
	vHOS_SPI_setByteDirection(	pxSdc->ucSpiUnitNumber,
								ucHOS_SPI_BYTE_DIRECTION_MSBYTE_FIRST	);
	vHOS_SPI_send(pxSdc->ucSpiUnitNumber, (int8_t*)&usCrc, 2);

	/*	Get data response	*/
	ucGotRd = ucHOS_SDC_getDataResponse(pxSdc, &xRd);
	if (!ucGotRd || xRd.ucStatus != SDC_Data_Response_Status_Accepted)
	{
//		vHOS_SPI_releaseMutex(pxSdc->ucSpiUnitNumber);
		return 0;
	}

//	vHOS_SPI_releaseMutex(pxSdc->ucSpiUnitNumber);

	return 1;
}

uint8_t ucHOS_SDC_keepTryingWriteBlock(	xHOS_SDC_t* pxSdc,
										xHOS_SDC_Block_Buffer_t* pxBlock,
										TickType_t xTimeout	)
{
	uint8_t ucSuccessfull;

	TickType_t xCurrentTime = xTaskGetTickCount();
	TickType_t xEndTime = xCurrentTime + xTimeout;

	if (xEndTime < xCurrentTime)
		xEndTime = portMAX_DELAY;

	while(xTimeout == portMAX_DELAY || xTaskGetTickCount() < xEndTime)
	{
		for (uint8_t i = 0; i < 3; i++)
		{
			ucSuccessfull = ucHOS_SDC_writeBlock(	pxSdc,
													pxBlock,
													xEndTime - xTaskGetTickCount()	);
			if (ucSuccessfull)
				return 1;
		}

		while(xTimeout == portMAX_DELAY || xTaskGetTickCount() < xEndTime)
		{
			ucSuccessfull = ucHOS_SDC_initFlow(pxSdc);
			if (ucSuccessfull)
				break;
		}
	}

	return 0;
}

uint8_t ucHOS_SDC_readBlock(	xHOS_SDC_t* pxSdc,
								xHOS_SDC_Block_Buffer_t* pxBlock,
								uint32_t uiBlockNumber,
								TickType_t xTimeout	)
{
	SDC_R1_t xR1;
	uint8_t ucGotR1;
	uint8_t ucSuccessful;
	uint8_t ucDummyByte;
	uint32_t uiAddress;

	/*	if the block requested is the one currently in buffer	*/
	if (uiBlockNumber == pxBlock->uiLbaRead && uiBlockNumber != 0)
		return 1;

	TickType_t xCurrentTime = xTaskGetTickCount();
	TickType_t xEndTime = xCurrentTime + pdMS_TO_TICKS(1000);
	if (xEndTime < xCurrentTime)
		xEndTime = portMAX_DELAY;

	/*	Acquire SPI mutex	*/
//	ucSuccessful = ucHOS_SPI_takeMutex(pxSdc->ucSpiUnitNumber, xTimeout);
//	if (!ucSuccessful)
//		return 0;

	/*	Get address (Based on SDC's version)	*/
	if (pxSdc->xVer == xHOS_SDC_Version_2_BlockAddress)
		uiAddress = uiBlockNumber;
	else if (pxSdc->xVer == xHOS_SDC_Version_2_ByteAddress)
		uiAddress = uiBlockNumber * 512;
	else
		uiAddress = 0;	/*	TODO: what does other versions (1, 3) do?	*/

	/*	Send CMD17	*/
	vHOS_SDC_sendCommand(pxSdc, 17, uiAddress);

	/*	Get R1 response	*/
	ucGotR1 = ucHOS_SDC_getR1(pxSdc, &xR1);
	if (ucGotR1 == 0 || ucHOS_SDC_checkR1(&xR1) == 0)
	{
//		vHOS_SPI_releaseMutex(pxSdc->ucSpiUnitNumber);
		return 0;
	}

	/*	wait for the data token (0b11111110) to be received	*/
	while(1)
	{
		vHOS_SPI_receive(pxSdc->ucSpiUnitNumber, (int8_t*)&ucDummyByte, 1);
		if (ucDummyByte == 0b11111110)
			break;

		if (xTaskGetTickCount() > xEndTime)
		{
			vHOS_SPI_releaseMutex(pxSdc->ucSpiUnitNumber);
			return 0;
		}
	}

	/*	Receive the data block	*/
	vHOS_SPI_setByteDirection(	pxSdc->ucSpiUnitNumber,
								ucHOS_SPI_BYTE_DIRECTION_LSBYTE_FIRST	);

	vHOS_SPI_receive(pxSdc->ucSpiUnitNumber, (int8_t*)pxBlock->pucBufferr, 512);

	/*	Receive the CRC	*/
	uint8_t pcCrcArr[2];

	vHOS_SPI_setByteDirection(	pxSdc->ucSpiUnitNumber,
								ucHOS_SPI_BYTE_DIRECTION_MSBYTE_FIRST	);

	vHOS_SPI_receive(pxSdc->ucSpiUnitNumber, pcCrcArr, 2);

	uint16_t usCrc = (pcCrcArr[1] << 8) | pcCrcArr[0];
	/*	Check CRC (if enabled)	*/
	if (pxSdc->ucIsCrcEnabled)
	{
		uint16_t usCrcCalc = usLIB_CRC_getCrc16(pxBlock->pucBufferr, 512);
		if (usCrcCalc != usCrc)
		{
//			vHOS_SPI_releaseMutex(pxSdc->ucSpiUnitNumber);
			return 0;
		}
	}

	pxBlock->uiLbaRead = uiBlockNumber;
	pxBlock->ucIsModified = 0;

//	vHOS_SPI_releaseMutex(pxSdc->ucSpiUnitNumber);

	return 1;
}

uint8_t ucHOS_SDC_keepTryingReadBlock(	xHOS_SDC_t* pxSdc,
										xHOS_SDC_Block_Buffer_t* pxBlock,
										uint32_t uiBlockNumber,
										TickType_t xTimeout	)
{
	uint8_t ucSuccessfull;

	TickType_t xCurrentTime = xTaskGetTickCount();
	TickType_t xEndTime = xCurrentTime + xTimeout;

	if (xEndTime < xCurrentTime)
		xEndTime = portMAX_DELAY;

	while(xTimeout == portMAX_DELAY || xTaskGetTickCount() < xEndTime)
	{
		for (uint8_t i = 0; i < 3; i++)
		{
			ucSuccessfull = ucHOS_SDC_readBlock(	pxSdc,
													pxBlock,
													uiBlockNumber,
													xEndTime - xTaskGetTickCount()	);
			if (ucSuccessfull)
				return 1;
		}

		while(xTimeout == portMAX_DELAY || xTaskGetTickCount() < xEndTime)
		{
			ucSuccessfull = ucHOS_SDC_initFlow(pxSdc);
			if (ucSuccessfull)
				break;
		}
	}

	return 0;
}
