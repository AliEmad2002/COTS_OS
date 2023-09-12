/*
 * SDC_CMD.c
 *
 *  Created on: Aug 16, 2023
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

/*	SELF	*/
#include "HAL/SDC/SDC_CMD.h"

/*
 * See header for info.
 */
void vHOS_SDC_sendCommand(xHOS_SDC_t* pxSdc, uint8_t ucIndex, uint32_t uiArg)
{
	// create command frame:
	uint8_t* pucArg = (uint8_t*)&uiArg;
	uint8_t pucCmdFrame[6] = {
		0b00000001, // CRC and stop bit (initially CRC is 0 as it is not yet calculated)
		pucArg[0],
		pucArg[1],
		pucArg[2],
		pucArg[3],
		0b01000000 | ucIndex	// index and start bits.
	};

	// calculate CRC and copy it to he frame:
	uint8_t ucCrc = ucLIB_CRC_getCrc7(&pucCmdFrame[1], 5);
	pucCmdFrame[0] |= (ucCrc << 1);

	// send it over SPI:
	vHOS_SPI_setByteDirection(	pxSdc->ucSpiUnitNumber,
								ucHOS_SPI_BYTE_DIRECTION_MSBYTE_FIRST	);

	vHOS_SPI_send(pxSdc->ucSpiUnitNumber, (int8_t*)pucCmdFrame, 6);
}

/*
 * See header for info.
 */
uint8_t ucHOS_SDC_getDataResponse(	xHOS_SDC_t* pxSdc,
									SDC_Data_Response_t* pxResponse	)
{
	uint8_t ucData;
	for (uint8_t i = 0; i < 8; i++)
	{
		vHOS_SPI_receive(pxSdc->ucSpiUnitNumber, (int8_t*)&ucData, 1);
		if (ucData != 0xFF)	// response start bit is received
		{
			*((uint8_t*)pxResponse) = ucData;
			break;
		}
	}

	/*	check start and end bits	*/
	if (pxResponse->ucStartBit != 1 || pxResponse->ucEndBit != 0)
		return 0;

	/*
	 * wait for busy flag to be cleared (MISO pin is driven low as long as the busy
	 * flag is set.
	 * TODO: is this necessary?
	 */
	//while(SPI_u8TransceiveData(sdc->spiUnitNumber, 0xFF) == 0);

	return 1;
}

/*
 * See header for info.
 */
uint8_t ucHOS_SDC_getR1(xHOS_SDC_t* pxSdc, SDC_R1_t* pxResponse)
{
	uint8_t ucData;
	for (uint8_t i = 0; i < 8; i++)
	{
		vHOS_SPI_receive(pxSdc->ucSpiUnitNumber, (int8_t*)&ucData, 1);
		if (ucData != 0xFF)	// response start bit is received
		{
			if (ucData != 0xFF)	// response start bit is received
			{
				*((uint8_t*)pxResponse) = ucData;
				break;
			}

			/*	check start bit	*/
			if (pxResponse->ucStartBit == 0)
				return 1;
		}
	}

	return 0;
}

uint8_t ucHOS_SDC_checkR1(SDC_R1_t* pxResponse)
{
	if (
		pxResponse->ucAddressErr	||
		pxResponse->ucCmdCrcErr  	||
		pxResponse->ucEraseSeqErr   ||
		pxResponse->ucIlligalCmdErr ||
		pxResponse->ucParameterErr
	)
	{
		return 0;
	}

	else
	{
		return 1;
	}
}

/*
 * See header for info.
 */
uint8_t ucHOS_SDC_getR3(xHOS_SDC_t* pxSdc, SDC_R3_t* pxResponse)
{
	// get R1 of this R3 response:
	uint8_t ucGotR1 = ucHOS_SDC_getR1(pxSdc, &pxResponse->xR1);
	if (!ucGotR1)
		return 0;

	// Check r1 for any errors:
	if (ucHOS_SDC_checkR1(&pxResponse->xR1) == 0)
		return 0;

	// get rest of R3 response (OCR value):
	vHOS_SPI_setByteDirection(	pxSdc->ucSpiUnitNumber,
								ucHOS_SPI_BYTE_DIRECTION_MSBYTE_FIRST	);
	vHOS_SPI_receive(pxSdc->ucSpiUnitNumber, (int8_t*)&pxResponse->xOcr, 4);

	return 1;
}

/*
 * See header for info.
 */
uint8_t ucHOS_SDC_getR7(xHOS_SDC_t* pxSdc, SDC_R7_t* pxResponse)
{
	// get R1 of this R7 response:
	uint8_t ucGotR1 = ucHOS_SDC_getR1(pxSdc, &pxResponse->xR1);
	if (!ucGotR1)
		return 0;

	// Check r1 for any errors:
	if (ucHOS_SDC_checkR1(&pxResponse->xR1) == 0)
		return 0;

	// get rest of R7 response:
	vHOS_SPI_setByteDirection(	pxSdc->ucSpiUnitNumber,
								ucHOS_SPI_BYTE_DIRECTION_MSBYTE_FIRST	);
	vHOS_SPI_receive(pxSdc->ucSpiUnitNumber, (int8_t*)&pxResponse->xCic, 4);

	return 1;
}

/*
 * See header for info.
 */
uint8_t ucHOS_SDC_sendAcmd(xHOS_SDC_t* pxSdc, uint8_t ucIndex, uint32_t uiArg)
{
	/*	send leading CMD55	*/
	vHOS_SDC_sendCommand(pxSdc, 55, 0);

	/*	Receive R1	*/
	SDC_R1_t xR1;
	uint8_t ucGotR1 = ucHOS_SDC_getR1(pxSdc, &xR1);
	if (!ucGotR1)
		return 0;

	if (xR1.ucInIdleState != 1 || ucHOS_SDC_checkR1(&xR1) == 0)
		return 0;

	/*	Now send command	*/
	vHOS_SDC_sendCommand(pxSdc, ucIndex, uiArg);

	return 1;
}

/*
 * See header for info.
 */
uint8_t ucHOS_SDC_getOcr(xHOS_SDC_t* pxSdc, SDC_OCR_t* pxOcr)
{
	SDC_R3_t xR3;
	uint8_t ucGotR3;

	/*	send CMD58	*/
	vHOS_SDC_sendCommand(pxSdc, 58, 0);

	/*	get response (R3)	*/
	ucGotR3 = ucHOS_SDC_getR3(pxSdc, &xR3);
	if (!ucGotR3)
		return 0;

	*pxOcr = xR3.xOcr;

	return 1;
}

/*
 * See header for info.
 */
uint8_t ucHOS_SDC_setBlockLen(xHOS_SDC_t* pxSdc, uint32_t uiLen)
{
	SDC_R1_t xR1;
	uint8_t ucGotR1;

	vHOS_SDC_sendCommand(pxSdc, 16, uiLen);

	/*	get response (R1)	*/
	ucGotR1 = ucHOS_SDC_getR1(pxSdc, &xR1);
	if (!ucGotR1)
		return 0;

	if (ucHOS_SDC_checkR1(&xR1) == 0)
		return 0;

	return 1;
}

/*
 * See header for info.
 */
uint8_t ucHOS_SDC_writeCrcEnable(xHOS_SDC_t* pxSdc, uint8_t ucCrcEnable)
{
	SDC_R1_t xR1;
	uint8_t ucGotR1;

	/*	Send CMD59 (Enable / Disable CRC)	*/
	pxSdc->ucIsCrcEnabled = ucCrcEnable;
	vHOS_SDC_sendCommand(pxSdc, 59, ucCrcEnable);

	/*	get response (R1)	*/
	ucGotR1 = ucHOS_SDC_getR1(pxSdc, &xR1);
	if (!ucGotR1)
		return 0;

	if (ucHOS_SDC_checkR1(&xR1) == 0)
		return 0;

	return 1;
}























