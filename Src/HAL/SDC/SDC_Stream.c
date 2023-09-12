/*
 * SDC_Stream.c
 *
 *  Created on: Aug 18, 2023
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
#include "HAL/SDC/SDC_IO.h"
#include "HAL/SDC/SDC_Dir.h"

/*	SELF	*/
#include "HAL/SDC/SDC_Stream.h"

/*
 * See header for info.
 */
uint8_t ucHOS_SDC_openStream(	xHOS_SDC_Stream_t* pxStream,
								char* pcFileName,
								TickType_t xTimeout	)
{
	uint8_t ucSuccessfull;
	SDC_DirData_t* pxDirData;
	uint8_t ucFound = 0;
	char pcInFileName[11];
	vHOS_SDC_getInFileName(pcInFileName, pcFileName);

	pxStream->uiReader = 0;
	pxStream->uiLastReader = 0;

	/*	Search for file's directory data record in the root directory	*/
	ucFound = ucHOS_SDC_findDirDataInDirectory(
		pxStream->pxSdc,
		pcInFileName,
		2,
		&pxDirData	);

	/*	if not found (TODO: create it)	*/
	if (!ucFound)
		return 0;

	/*	Get size of the found file	*/
	pxStream->uiSizeActual = pxDirData->uiFileSize;
	pxStream->uiSizeOnSDC = pxDirData->uiFileSize;

	/*	Get first cluster number of the found file	*/
	pxStream->uiFirstClusterNumber =
		((uint32_t)pxDirData->usFirstClusterHigh << 16) |
		(uint32_t)pxDirData->usFirstClusterLow;

	/*	Get LBA of this cluster	*/
	pxStream->uiStartLba = uiHOS_SDC_getClusterLba(
		pxStream->pxSdc,
		pxStream->uiFirstClusterNumber	);

	/*	read first sector of file's first cluster into stream object buffer	*/
	ucSuccessfull = ucHOS_SDC_readBlock(	pxStream->pxSdc,
											&pxStream->xBuffer,
											pxStream->uiStartLba,
											xTimeout	);
	if (!ucSuccessfull)
		return 0;

	/*	stream opened	*/
	return 1;
}

/*
 * See header for info.
 */
uint8_t ucHOS_SDC_keepTryingOpenStream(	xHOS_SDC_Stream_t* pxStream,
										char* pcFileName,
										TickType_t xTimeout	)
{
	uint8_t ucSuccessfull;

	TickType_t xEndTime = xTaskGetTickCount() + xTimeout;

	while(xTaskGetTickCount() < xEndTime)
	{
		for (uint8_t i = 0; i < 3; i++)
		{
			ucSuccessfull = ucHOS_SDC_openStream(
				pxStream,
				pcFileName,
				xEndTime - xTaskGetTickCount()	);

			if (ucSuccessfull)
				return 1;
		}

		while(xTaskGetTickCount() < xEndTime)
		{
			ucSuccessfull = ucHOS_SDC_initFlow(pxStream->pxSdc);
			if (ucSuccessfull)
				break;
		}
	}

	return 0;
}

/*
 * See header for info.
 */
uint8_t ucHOS_SDC_saveCurrentBuffer(	xHOS_SDC_Stream_t* pxStream,
										TickType_t xTimeout	)
{
	uint8_t ucSuccessfull;

	/*	Get cluster index of the sector to be buffered	*/
	uint32_t uiBufferOffset = pxStream->xBuffer.uiLbaRead - pxStream->uiStartLba;

	uint32_t uiClusterIndex =
		uiBufferOffset / pxStream->pxSdc->ucSectorsPerCluster;

	/*	Get cluster number of this cluster index	*/
	uint32_t uiClusterNumber = uiHOS_SDC_getClusterNumber(
		pxStream->pxSdc, pxStream->uiFirstClusterNumber, uiClusterIndex	);

	if (uiClusterNumber == 0xFFFFFFFF)
		return 0;

	/*	Write the current buffer to the SD-card	*/
	ucSuccessfull = ucHOS_SDC_writeBlock(
		pxStream->pxSdc,
		&pxStream->xBuffer,
		xTimeout);

	if (!ucSuccessfull)
		return 0;

	return 1;
}

/*
 * See header for info.
 */
uint8_t ucHOS_SDC_readSector(	xHOS_SDC_Stream_t* pxStream,
								uint32_t uiSectorsOffset,
								TickType_t xTimeout	)
{
	uint8_t uiSuccessfull;

	/*	Get cluster index of the sector to be buffered	*/
	uint32_t uiClusterIndex =
		uiSectorsOffset / pxStream->pxSdc->ucSectorsPerCluster;

	/*	Get cluster number of this cluster index	*/
	uint32_t uiClusterNumber = uiHOS_SDC_getClusterNumber(
		pxStream->pxSdc,
		pxStream->uiFirstClusterNumber,
		uiClusterIndex	);

	if (uiClusterNumber == 0xFFFFFFFF)
		return 0;

	/*	Get LBA of the sector to be buffered	*/
	uint32_t uiLba =
		uiHOS_SDC_getClusterLba(pxStream->pxSdc, uiClusterNumber) +
		uiSectorsOffset % pxStream->pxSdc->ucSectorsPerCluster;

	/*	Read that sector	*/
	uiSuccessfull = ucHOS_SDC_readBlock(	pxStream->pxSdc,
											&pxStream->xBuffer,
											uiLba,
											xTimeout	);
	if (!uiSuccessfull)
		return 0;

	return 1;
}

/*
 * See header for info.
 */
uint8_t ucHOS_SDC_updateBbuffer(	xHOS_SDC_Stream_t* pxStream,
									uint32_t uiOffset,
									TickType_t xTimeout	)
{
	uint8_t ucSuccessfull;

	/*	if the given offset is outside the sector currently in buffer	*/
	uint32_t uiSectorsOffset = uiOffset / 512;

	uint32_t uiStreamBufferOffset =
		pxStream->xBuffer.uiLbaRead - pxStream->uiStartLba;

	if (uiSectorsOffset != uiStreamBufferOffset)
	{
		/*	save the current buffer to SD-card	*/
		ucSuccessfull = ucHOS_SDC_saveCurrentBuffer(pxStream, xTimeout);
		if (!ucSuccessfull)
			return 0;

		/*	Read new buffer from SD-card	*/
		ucSuccessfull = ucHOS_SDC_readSector(pxStream, uiSectorsOffset, xTimeout);
		if (!ucSuccessfull)
			return 0;
	}

	return 1;
}

/*
 * See header for info.
 */
uint8_t ucHOS_SDC_keepTryingUpdateBuffer(	xHOS_SDC_Stream_t* pxStream,
											uint32_t uiOffset,
											TickType_t xTimeout	)
{
	uint8_t ucSuccessfull;

	TickType_t xEndTime = xTaskGetTickCount() + xTimeout;

	while(xTaskGetTickCount() < xEndTime)
	{
		for (uint8_t i = 0; i < 3; i++)
		{
			ucSuccessfull = ucHOS_SDC_updateBbuffer(
				pxStream,
				uiOffset,
				xEndTime - xTaskGetTickCount()	);

			if (ucSuccessfull)
				return 1;
		}

		while(xTaskGetTickCount() < xEndTime)
		{
			ucSuccessfull = ucHOS_SDC_initFlow(pxStream->pxSdc);
			if (ucSuccessfull)
				break;
		}
	}

	return 0;
}

/*
 * See header for info.
 */
uint8_t ucHOS_SDC_readStream(	xHOS_SDC_Stream_t* pxStream,
								uint32_t uiOffset,
								uint8_t* pucArr,
								uint32_t uiLen,
								TickType_t xTimeout	)
{
	uint8_t ucSccessfull;

	TickType_t xEndTime = xTaskGetTickCount() + xTimeout;

	ucSccessfull = ucHOS_SDC_updateBbuffer(pxStream, uiOffset, xTimeout);
	if (!ucSccessfull)
		return 0;

	/*
	 * Program will copy from "stream->buffer" and break when it reaches its end,
	 * then, if the required length is not yet copied, an "update_buffer()" operation
	 * will take place before continuing.
	 */
	uint32_t i = 0;
	while(1)
	{
		if (i == uiLen)
			return 1;

		if ((uiOffset+i) % 512 == 0)
		{
			ucSccessfull = ucHOS_SDC_updateBbuffer(
				pxStream,
				uiOffset + i,
				xEndTime - xTaskGetTickCount());

			if (!ucSccessfull)
				return 0;
		}

		pucArr[i] = pxStream->xBuffer.pucBufferr[(uiOffset+i) % 512];
		i++;
	}
}

/*
 * See header for info.
 */
uint8_t ucHOS_SDC_keepTryingReadStream(	xHOS_SDC_Stream_t* pxStream,
										uint32_t uiOffset,
										uint8_t* pucArr,
										uint32_t uiLen,
										TickType_t xTimeout	)
{
	uint8_t ucSuccessfull;

	TickType_t xEndTime = xTaskGetTickCount() + xTimeout;

	while(xTaskGetTickCount() < xEndTime)
	{
		for (uint8_t i = 0; i < 3; i++)
		{
			ucSuccessfull = ucHOS_SDC_readStream(
				pxStream,
				uiOffset,
				pucArr,
				uiLen,
				xEndTime - xTaskGetTickCount()	);

			if (ucSuccessfull)
				return 1;
		}

		while(xTaskGetTickCount() < xEndTime)
		{
			ucSuccessfull = ucHOS_SDC_initFlow(pxStream->pxSdc);
			if (ucSuccessfull)
				break;
		}
	}

	return 0;
}

/*
 * See header for info.
 */
uint8_t ucHOS_SDC_writeStream(	xHOS_SDC_Stream_t* pxStream,
								uint32_t uiOffset,
								uint8_t* pucArr,
								uint32_t uiLen,
								TickType_t xTimeout	)
{
	/*
	 * TODO: if written with more than original size, allocate additional
	 * cluster and update size field in the data record on the card.
	 */
	uint8_t ucSuccessfull;

	TickType_t xEndTime = xTaskGetTickCount() + xTimeout;

	ucSuccessfull = ucHOS_SDC_updateBbuffer(pxStream, uiOffset, xTimeout);
	if (!ucSuccessfull)
		return 0;

	/*
	 * Program will copy to "stream->buffer" and break when it reaches its end,
	 * then, if the required length is not yet copied, an "update_buffer()" operation
	 * will take place before continuing.
	 */
	uint32_t i = 0;
	while(1)
	{
		if (i == uiLen)
			return 1;

		if ((uiOffset+i) % 512 == 0)
		{
			ucSuccessfull = ucHOS_SDC_updateBbuffer(
				pxStream,
				uiOffset + i,
				xEndTime - xTaskGetTickCount());

			if (!ucSuccessfull)
				return 0;
		}

		pxStream->xBuffer.pucBufferr[(uiOffset+i) % 512] = pucArr[i];
		i++;
	}
}

/*
 * See header for info.
 */
uint8_t ucHOS_SDC_keepTryingWriteStream(	xHOS_SDC_Stream_t* pxStream,
											uint32_t uiOffset,
											uint8_t* pucArr,
											uint32_t uiLen,
											TickType_t xTimeout	)
{
	uint8_t ucSuccessfull;

	TickType_t xEndTime = xTaskGetTickCount() + xTimeout;

	while(xTaskGetTickCount() < xEndTime)
	{
		for (uint8_t i = 0; i < 3; i++)
		{
			ucSuccessfull = ucHOS_SDC_writeStream(
				pxStream,
				uiOffset,
				pucArr,
				uiLen,
				xEndTime - xTaskGetTickCount()	);

			if (ucSuccessfull)
				return 1;
		}

		while(xTaskGetTickCount() < xEndTime)
		{
			ucSuccessfull = ucHOS_SDC_initFlow(pxStream->pxSdc);
			if (ucSuccessfull)
				break;
		}
	}

	return 0;
}

/*
 * See header for info.
 */
uint8_t ucHOS_SDC_saveStream(xHOS_SDC_Stream_t* pxStream, TickType_t xTimeout)
{
	uint8_t ucSuccessfull;

	/*	save the current buffer to SD-card	*/
	ucSuccessfull = ucHOS_SDC_saveCurrentBuffer(pxStream, xTimeout);
	if (!ucSuccessfull)
		return 0;

	return 1;
}

/*
 * See header for info.
 */
uint8_t ucHOS_SDC_keepTryingSaveStream(	xHOS_SDC_Stream_t* pxStream,
										TickType_t xTimeout	)
{
	uint8_t ucSuccessfull;

	TickType_t xEndTime = xTaskGetTickCount() + xTimeout;

	while(xTaskGetTickCount() < xEndTime)
	{
		for (uint8_t i = 0; i < 3; i++)
		{
			ucSuccessfull = ucHOS_SDC_saveStream(
				pxStream,
				xEndTime - xTaskGetTickCount()	);

			if (ucSuccessfull)
				return 1;
		}

		while(xTaskGetTickCount() < xEndTime)
		{
			ucSuccessfull = ucHOS_SDC_initFlow(pxStream->pxSdc);
			if (ucSuccessfull)
				break;
		}
	}

	return 0;
}

/*
 * See header for info.
 */
uint8_t ucHOS_SDC_getNextLine(	xHOS_SDC_Stream_t* pxStream,
								char* pcLine,
								uint32_t uiMaxSize,
								TickType_t xTimeout	)
{
	uint8_t ucSuccessfull;
	uint32_t uiOffset = pxStream->uiReader;
	uint32_t i = 0;

	TickType_t xEndTime = xTaskGetTickCount() + xTimeout;

	ucSuccessfull = ucHOS_SDC_keepTryingUpdateBuffer(pxStream, uiOffset,xTimeout);
	if (!ucSuccessfull)
		return 0;

	while(1)
	{
		/*	if max size is reached, or stream file has ended	*/
		if ( (i >= uiMaxSize) || (uiOffset+i >= pxStream->uiSizeActual) )
		{
			pcLine[i] = '\0';
			pxStream->uiLastReader = pxStream->uiReader;
			pxStream->uiReader = uiOffset + i;
			return 0;
		}

		/*	if going to start reading from next sector, update buffer	*/
		if ((uiOffset+i) % 512 == 0)
		{
			ucSuccessfull = ucHOS_SDC_keepTryingUpdateBuffer(
				pxStream,
				uiOffset+i,
				xEndTime - xTaskGetTickCount()	);

			if (!ucSuccessfull)
				return 0;
		}

		/*	Copy byte	*/
		pcLine[i] = pxStream->xBuffer.pucBufferr[(uiOffset+i) % 512];

		/*	check end of line	*/
		if (pcLine[i] == '\n')
		{
			if (i > 0)
			{
				if (pcLine[i - 1] == '\r')	// remove unnecessary '\r'
					pcLine[i - 1] = '\0';
			}
			pcLine[i] = '\0';
			pxStream->uiLastReader = pxStream->uiReader;
			pxStream->uiReader = uiOffset + i + 1;
			break;
		}

		i++;
	}

	return 1;
}

/*
 * See header for info.
 */
void vHOS_SDC_resetLineReader(xHOS_SDC_Stream_t* pxStream)
{
	pxStream->uiReader = 0;
	pxStream->uiLastReader = 0;
}

/*
 * See header for info.
 */
uint8_t ucHOS_SDC_isThereNextLine(xHOS_SDC_Stream_t* pxStream)
{
	return (pxStream->uiReader < pxStream->uiSizeActual);
}

/*
 * See header for info.
 */
uint8_t ucHOS_SDC_seekReaderPrevLine(xHOS_SDC_Stream_t* pxStream)
{
	/*	if this is the second seek in a row	*/
	if (pxStream->uiReader == pxStream->uiLastReader)
		return 0;

	/*	Otherwise	*/
	pxStream->uiReader = pxStream->uiLastReader;
	return 1;
}




