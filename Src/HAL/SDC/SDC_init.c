/*
 * SDC_init.c
 *
 *  Created on: Aug 17, 2023
 *      Author: Ali Emad
 */

/*	LIB	*/
#include "stdint.h"

/*	RTOS	*/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/*	MCAL	*/
#include "MCAL_Port/Port_SPI.h"
#include "MCAL_Port/Port_DIO.h"

/*	HAL	*/
#include "HAL/SPI/SPI.h"
#include "HAL/SDC/SDC.h"
#include "HAL/SDC/SDC_Private.h"
#include "HAL/SDC/SDC_CMD.h"
#include "HAL/SDC/SDC_IO.h"
#include "HAL/SDC/SDC.h"

/*	SELF	*/
#include "HAL/SDC/SDC_init.h"

/*
 * See header for info.
 */
xHOS_SDC_Version_t xHOS_SDC_initBranch2(xHOS_SDC_t* pxSdc)
{
	SDC_R1_t xR1;
	uint8_t ucGotR1;

	TickType_t xStartTime = xTaskGetTickCount();

	while(
			xTaskGetTickCount() - xStartTime <
			pdMS_TO_TICKS(configHOS_SDC_INIT_LOOP_TIMEOUT_MS)	)
	{
		/*	send CMD1	*/
		vHOS_SDC_sendCommand(pxSdc, 1, 0);


		/*	get response (R1)	*/
		ucGotR1 = ucHOS_SDC_getR1(pxSdc, &xR1);
		if (ucGotR1 == 0 || ucHOS_SDC_checkR1(&xR1) == 0)
			return xHOS_SDC_Version_Unknown;

		if (xR1.ucInIdleState == 1) // card needs more time:
			continue;

		else	// card has received and processed the ACMD41 successfully
			return xHOS_SDC_Version_3;
	}

	// if timeout period passed:
	return xHOS_SDC_Version_Unknown;
}

/*
 * See header for info.
 */
xHOS_SDC_Version_t xHOS_SDC_initBranch1(xHOS_SDC_t* pxSdc)
{
	SDC_R1_t xR1;
	uint8_t ucGotR1;
	uint8_t ucSuccessfull;

	TickType_t xStartTime = xTaskGetTickCount();

	while(
			xTaskGetTickCount() - xStartTime <
			pdMS_TO_TICKS(configHOS_SDC_INIT_LOOP_TIMEOUT_MS)	)
	{
		/*	send ACMD41	*/
		ucSuccessfull = ucHOS_SDC_sendAcmd(pxSdc, 41, 0);
		if (!ucSuccessfull)
			return xHOS_SDC_Version_Unknown;

		/*	get response (R1)	*/
		ucGotR1 = ucHOS_SDC_getR1(pxSdc, &xR1);
		if (!ucGotR1)	// if no response:
			return xHOS_SDC_initBranch2(pxSdc);

		else if (ucHOS_SDC_checkR1(&xR1) == 0)	// if error response:
			return xHOS_SDC_initBranch2(pxSdc);

		if (xR1.ucInIdleState == 1) // card needs more time:
			continue;

		else	// card has received and processed the ACMD41 successfully
			return xHOS_SDC_Version_1;
	}

	// if timeout period passed:
	return xHOS_SDC_initBranch2(pxSdc);
}

/*
 * See header for info.
 */
xHOS_SDC_Version_t xHOS_SDC_initBranch0(xHOS_SDC_t* pxSdc)
{
	SDC_R1_t xR1;
	uint8_t ucGotR1;
	uint8_t ucSuccessfull;
	xHOS_SDC_Version_t xVer = xHOS_SDC_Version_Unknown;

	TickType_t xEndTime = 	xTaskGetTickCount() +
							pdMS_TO_TICKS(configHOS_SDC_INIT_LOOP_TIMEOUT_MS);

	while(xTaskGetTickCount() < xEndTime)
	{
		/*	send ACMD41	*/
		ucSuccessfull = ucHOS_SDC_sendAcmd(pxSdc, 41, 1ul << 30);
		if (!ucSuccessfull)
			break;

		/*	get response (R1)	*/
		ucGotR1 = ucHOS_SDC_getR1(pxSdc, &xR1);
		if (ucGotR1 == 0 || ucHOS_SDC_checkR1(&xR1) == 0)	// if no response or error response:
			break;

		if (xR1.ucInIdleState == 1) // card needs more time:
			continue;

		else	// card has received and processed the ACMD41 successfully
		{
			/*	read card's OCR	*/
			SDC_OCR_t xOcr;
			ucSuccessfull = ucHOS_SDC_getOcr(pxSdc, &xOcr);
			if (!ucSuccessfull)
				break;

			if (xOcr.uiCcs == 0)
				xVer = xHOS_SDC_Version_2_ByteAddress;
			else
				xVer = xHOS_SDC_Version_2_BlockAddress;

			break;
		}
	}

	return xVer;
}

/*
 * See header for info.
 */
uint8_t ucHOS_SDC_initFlow(xHOS_SDC_t* pxSdc)
{
	SDC_R1_t xR1;
	SDC_R7_t xR7;
	uint8_t ucGotR1;
	uint8_t ucGotR7;
	uint8_t ucSuccessfull;
	uint8_t ucDummyByte = 0xFF;

	/*	Acquire SPI mutex	*/
	ucSuccessfull = ucHOS_SPI_takeMutex(
						pxSdc->ucSpiUnitNumber,
						pdMS_TO_TICKS(configHOS_SDC_INIT_LOOP_TIMEOUT_MS)	);
	if (!ucSuccessfull)
		return 0;

	vPort_SPI_setBaudratePrescaler(	pxSdc->ucSpiUnitNumber,
									configHOS_SDC_INIT_SPI_BAUD_PRESCALER	);

	/*	Initially, no sectors have been read yet	*/
	pxSdc->xBuffer.uiLbaRead = 0xFFFFFFFF;

	/**	Initialization flow: (Diagram is at the directory: ../Inc/HAL/SDC)	**/

	/*	select chip	*/
	vPORT_DIO_WRITE_PIN(pxSdc->ucCsPort, pxSdc->ucCsPin, 1);

	vTaskDelay(pdMS_TO_TICKS(2));

	/*	>= 74 dummy clocks	*/
	vHOS_SPI_sendMultiple(pxSdc->ucSpiUnitNumber, (int8_t*)&ucDummyByte, 1, 10);

	/*	select chip	*/
	vPORT_DIO_WRITE_PIN(pxSdc->ucCsPort, pxSdc->ucCsPin, 0);

	/*	send CMD0	*/
	vHOS_SDC_sendCommand(pxSdc, 0, 0);

	/*	get response (R1)	*/
	ucGotR1 = ucHOS_SDC_getR1(pxSdc, &xR1);
	if (ucGotR1 == 0 || ucHOS_SDC_checkR1(&xR1) == 0)	// if no response or error response:
		return 0;

	/*	send CMD8	*/
	vHOS_SDC_sendCommand(pxSdc, 8, 0x000001AA);

	/*	get response (R7)	*/
	ucGotR7 = ucHOS_SDC_getR7(pxSdc, &xR7);

	if (!ucGotR7)	// if no response:
		pxSdc->xVer = xHOS_SDC_initBranch1(pxSdc);

	else if (xR7.xCic.ucCheckPattern == 0xAA && xR7.xCic.ucVoltageAccepted == 1)
		pxSdc->xVer = xHOS_SDC_initBranch0(pxSdc);

	else	// mismatch
		pxSdc->xVer = xHOS_SDC_Version_Unknown;

	/*	if version is unknown	*/
	if (pxSdc->xVer == xHOS_SDC_Version_Unknown)
		return 0;

	/*	if version is known and other than "SDC_Version_2_BlockAddress"	*/
	if (pxSdc->xVer != xHOS_SDC_Version_2_BlockAddress)
	{
		/*	set block len to 512 bytes (CMD16)	*/
		ucSuccessfull = ucHOS_SDC_setBlockLen(pxSdc, 512);
		if (!ucSuccessfull)
			return 0;
	}

	/*	Enable / Disable CRC	*/
	ucSuccessfull = ucHOS_SDC_writeCrcEnable(pxSdc, pxSdc->ucIsCrcEnabled);
	if (!ucSuccessfull)
		return 0;

	vPort_SPI_setBaudratePrescaler(	pxSdc->ucSpiUnitNumber,
									configHOS_SDC_TRANS_SPI_BAUD_PRESCALER	);

	vHOS_SPI_releaseMutex(pxSdc->ucSpiUnitNumber);

	return 1;
}

/*
 * See header for info.
 */
uint8_t ucHOS_SDC_initPartition(xHOS_SDC_t* pxSdc, TickType_t xTimeout)
{
	uint8_t ucSuccessfull;

	static uint8_t ucFirstTime = 1;
	if (ucFirstTime)
	{
		ucSuccessfull = ucHOS_SDC_initFlow(pxSdc);
		if (!ucSuccessfull)
			return 0;
	}

	/*	Read zero-th sector (MBR)	*/
	ucSuccessfull = ucHOS_SDC_readBlock(pxSdc, &pxSdc->xBuffer, 0, xTimeout);
	if (!ucSuccessfull)
		return 0;

	/*	In the partition table, search for the partition that uses FAT32	*/
	SDC_Partition_Entry_t* pxPartitionEntry;
	for (uint8_t i = 0; i < 4; i++)
	{
		pxPartitionEntry =
			(SDC_Partition_Entry_t*)&(pxSdc->xBuffer.pucBufferr[446 + 16 * i]);

		if (	pxPartitionEntry->ucTypeCode == 0xB ||
				pxPartitionEntry->ucTypeCode == 0xC	)
		{
			break;
		}
	}

	/*	Read volume ID sector of the partition (first sector in partition)	*/
	uint32_t uiLbaBegin = pxPartitionEntry->uiLbaBegin;
	ucSuccessfull =
		ucHOS_SDC_readBlock(pxSdc, &pxSdc->xBuffer, uiLbaBegin, xTimeout);
	if (!ucSuccessfull)
		return 0;

	uint16_t usBytesPerSector			= *(uint16_t*)&(pxSdc->xBuffer.pucBufferr[0x0B]);
	pxSdc->ucSectorsPerCluster			= *(uint8_t*)&(pxSdc->xBuffer.pucBufferr[0x0D]);
	uint16_t usNumberOfReservedSectors	= *(uint16_t*)&(pxSdc->xBuffer.pucBufferr[0x0E]);
	uint8_t ucNumberOfFats				= *(uint8_t*)&(pxSdc->xBuffer.pucBufferr[0x10]);
	pxSdc->xFat.uiSectorsPerFat			= *(uint32_t*)&(pxSdc->xBuffer.pucBufferr[0x24]);
	uint16_t usSignature				= *(uint16_t*)&(pxSdc->xBuffer.pucBufferr[0x1FE]);

	/*	Check constant values (from "volume ID critical fields" table in the document)	*/
	if (usBytesPerSector != 512)
		return 0;
	if (ucNumberOfFats != 2)
		return 0;
	if (usSignature != 0xAA55)
		return 0;

	/*	Get LBA of the File Allocation Table of that partition	*/
	pxSdc->xFat.uiLba = uiLbaBegin + usNumberOfReservedSectors;

	/*	Get LBA of the first cluster in root directory	*/
	pxSdc->uiClustersBeginLba =
		pxSdc->xFat.uiLba + ucNumberOfFats * pxSdc->xFat.uiSectorsPerFat;

	if (ucFirstTime)
	{
		xSemaphoreGive(pxSdc->xInitCompletionSemaphore);
		ucFirstTime = 0;
	}

	return 1;
}

uint8_t ucHOS_SDC_keepTryingInitPartition(xHOS_SDC_t* pxSdc, TickType_t xTimeout)
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
			ucSuccessfull =
				ucHOS_SDC_initPartition(	pxSdc,
											xEndTime - xTaskGetTickCount()	);
			if (ucSuccessfull)
				return 1;
		}

		while(xTaskGetTickCount() < xEndTime)
		{
			ucSuccessfull = ucHOS_SDC_initFlow(pxSdc);
			if (ucSuccessfull)
				break;
		}
	}

	return 0;
}

void vHOS_SDC_init(xHOS_SDC_t* pxSdc)
{
	/*	Initialize CS pin	*/
	vPort_DIO_initPinOutput(pxSdc->ucCsPort, pxSdc->ucCsPin);

	/*	Initialize SDC handle's mutex	*/
	pxSdc->xMutex = xSemaphoreCreateMutexStatic(&pxSdc->xMutexStatic);
	xSemaphoreGive(pxSdc->xMutex);

	/*	Initialize initialization completion semaphore	*/
	pxSdc->xInitCompletionSemaphore =
		xSemaphoreCreateBinaryStatic(&pxSdc->xInitCompletionSemaphoreStatic);
	xSemaphoreTake(pxSdc->xInitCompletionSemaphore, 0);
}

uint8_t ucHOS_SDC_waitForInitCompletion(xHOS_SDC_t* pxSdc, TickType_t xTimeout)
{
	/*	Try taking the semaphore in the given timeout	*/
	uint8_t ucState = xSemaphoreTake(pxSdc->xInitCompletionSemaphore, xTimeout);

	/*
	 * If semaphore was taken successfully, release it back so other tasks can
	 * also synchronize.
	 */
	if (ucState)
	{
		xSemaphoreGive(pxSdc->xInitCompletionSemaphore);
	}

	/*	Return status of init completion	*/
	return ucState;
}






