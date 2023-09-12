/*
 * SDC_Dir.c
 *
 *  Created on: Aug 18, 2023
 *      Author: Ali Emad
 */

/*	LIB	*/
#include "stdint.h"
#include "ctype.h"

/*	RTOS	*/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/*	HAL	*/
#include "HAL/SDC/SDC.h"
#include "HAL/SDC/SDC_Private.h"
#include "HAL/SDC/SDC_CMD.h"
#include "HAL/SDC/SDC_IO.h"

/*	SELF	*/
#include "HAL/SDC/SDC_Dir.h"

/*
 * See header for info.
 */
SDC_DirRecordType_t xHOS_SDC_getDirRecordType(SDC_DirData_t* pxRec)
{
	/*	Normal	*/
	if (!pxRec->xAttrib.ucVolumeId  &&
		!pxRec->xAttrib.ucDirectory &&
		!pxRec->xAttrib.ucUnused0   &&
		!pxRec->xAttrib.ucUnused1
	)
		return SDC_DirRecordType_Normal;

	/*	Long file name	*/
	if (pxRec->xAttrib.ucSystem    &&
		pxRec->xAttrib.ucVolumeId  &&
		pxRec->xAttrib.ucDirectory &&
		pxRec->xAttrib.ucArchive   &&
		!pxRec->xAttrib.ucUnused0  &&
		!pxRec->xAttrib.ucUnused1
	)
		return SDC_DirRecordType_LongFileName;

	/*	unused	*/
	if (pxRec->pcShortFileName[0] == (char)0xE5)
		return SDC_DirRecordType_Unused;

	/*	End of directory	*/
	if (pxRec->pcShortFileName[0] == 0)
		return SDC_DirRecordType_EndOfDir;

	/*	Otherwise, unknown type	*/
	return SDC_DirRecordType_Unknown;
}

/*
 * See header for info.
 */
void vHOS_SDC_getInFileName(char pcInFileName[11], char* pcFileName)
{
	/*	copy from first to the '.' or the end of "fileNme" if '.' does not exist	*/
	int8_t cDotIndex = -1;
	uint8_t i = 0;
	for (; ; i++)
	{
		if (pcFileName[i] == '.')
		{
			cDotIndex = i;
			break;
		}

		if (pcFileName[i] == '\0')
			break;

		pcInFileName[i] = toupper(pcFileName[i]);
	}

	/*	if '.' does not exist	*/
	if (cDotIndex == -1)
	{
		/*	fill rest of "inFileName" with spaces	*/
		for (; i < 11; i++)
			pcInFileName[i] = ' ';
		/*	end function	*/
		return;
	}

	/*	otherwise, if '.' exists, add spaces until last 3 bytes	*/
	for (; i < 8; i++)
		pcInFileName[i] = ' ';

	/*	copy extension	*/
	for (i = 0; i < 3; i++)
	{
		if (pcFileName[cDotIndex + 1 + i] == '\0')
			break;
		pcInFileName[8 + i] = toupper(pcFileName[cDotIndex + 1 + i]);
	}

	/*	fill rest of "inFileName" with spaces (some extensions are of 1 or two bytes only)	*/
	for (; i < 3; i++)
		pcInFileName[8 + i] = ' ';
}

/*
 * See header for info.
 */
uint8_t ucHOS_SDC_areEqualNames(char* pcName1, char* pcName2)
{
	for (uint8_t i =0; i < 11; i++)
	{
		if (pcName1[i] != pcName2[i])
			return 0;
	}
	return 1;
}

/*
 * See header for info.
 */
uint32_t uiHOS_SDC_getClusterLba(xHOS_SDC_t* pxSdc, uint32_t uiClusterNumber)
{
	return	pxSdc->uiClustersBeginLba +
			(uiClusterNumber - 2) * pxSdc->ucSectorsPerCluster	;
}

/*
 * See header for info.
 */
uint8_t ucHOS_SDC_findDirDataInCurrentSector(	xHOS_SDC_t* pxSdc,
												char* pcInFileName,
												SDC_DirData_t** ppxDirData	)
{
	/*	List all files in the current sector and search for "inFileName" among them	*/
	for (uint16_t i = 0; i < 16; i++)
	{
		/*	get pointer to the i-th record	*/
		*ppxDirData = (SDC_DirData_t*)&(pxSdc->xBuffer.pucBufferr[32 * i]);
		/*	get type of this record	*/
		SDC_DirRecordType_t xRecType = xHOS_SDC_getDirRecordType(*ppxDirData);
		/*	check for end of directory	*/
		if (xRecType == SDC_DirRecordType_EndOfDir)
			return 2;
		/*	check for short file name	*/
		if (xRecType != SDC_DirRecordType_Normal)
			continue;
		/*	if "dirData" record expresses a short named file, compare it with the given "fileName"	*/
		if (ucHOS_SDC_areEqualNames(pcInFileName, (*ppxDirData)->pcShortFileName))
			return 1;
	}

	return 0;
}

/*
 * See header for info.
 */
uint8_t ucHOS_SDC_findDirDataInCluster(	xHOS_SDC_t* pxSdc,
										char* pcInFileName,
										uint32_t uiClusterNumber,
										SDC_DirData_t** ppxDirData	)
{
	uint8_t ucSuccessfull;
	uint8_t ucFound;

	/*	Get LBA of that cluster	*/
	uint32_t uiClusterLba = uiHOS_SDC_getClusterLba(pxSdc, uiClusterNumber);

	/*	For every sector in the cluster	*/
	for (uint8_t iSector = 0; iSector < pxSdc->ucSectorsPerCluster; iSector++)
	{
		/*	Read that sector into the buffer	*/
		ucSuccessfull = ucHOS_SDC_readBlock(
			pxSdc,
			&pxSdc->xBuffer,
			uiClusterLba + iSector,
			portMAX_DELAY	);

		if (!ucSuccessfull)
			return 0;

		/*	Search in this sector	*/
		ucFound = ucHOS_SDC_findDirDataInCurrentSector(pxSdc, pcInFileName, ppxDirData);

		/*	if not found but containing directory has not yet ended	*/
		if (ucFound == 0)
			continue;

		/*	if found	*/
		if (ucFound == 1)
			return 1;

		/*	if not found and containing directory has ended	*/
		if (ucFound == 2)
			return 2;
	}

	/*	Cluster ended and file has not been found, and also, directory has not yet ended	*/
	return 0;
}

/*
 * See header for info.
 */
uint32_t uiHOS_SDC_getNextClusterNumber(	xHOS_SDC_t* pxSdc,
											uint32_t uiCurrentClusterNumber	)
{
	/*	Get next cluster number by reading FAT's integer indexed by "uiCurrentClusterNumber"	*/
	/*	Read FAT (File Allocation Table) sector containing that entry	*/
	ucHOS_SDC_keepTryingReadBlock(
		pxSdc,
		&pxSdc->xBuffer,
		pxSdc->xFat.uiLba + uiCurrentClusterNumber / 128,
		portMAX_DELAY	);

	/*	if there's no next cluster	*/
	uint32_t uiEntry =
		((uint32_t*)pxSdc->xBuffer.pucBufferr)[uiCurrentClusterNumber % 128];
	if (uiEntry >= 0xFFFFFFF8 || uiEntry == 0x0FFFFFFF)
		return 0xFFFFFFFF;

	/*	clear upper 4-bits	*/
	uint32_t uiNextClusterNumber = uiEntry & 0x0FFFFFFFF;

	return uiNextClusterNumber;
}

/*
 * See header for info.
 */
uint8_t ucHOS_SDC_findDirDataInDirectory(	xHOS_SDC_t* pxSdc,
											char* pcInFileName,
											uint32_t uiDirFirstClusterNumber,
											SDC_DirData_t** dirDataPP	)
{
	uint8_t ucFound;

	/*	For every cluster in the directory	*/
	uint32_t uiCurrentClusterNumber = uiDirFirstClusterNumber;
	while(uiCurrentClusterNumber != 0xFFFFFFFF)
	{
		/*	Search in this cluster	*/
		ucFound = ucHOS_SDC_findDirDataInCluster(	pxSdc,
													pcInFileName,
													uiCurrentClusterNumber,
													dirDataPP	);

		/*	if found	*/
		if (ucFound == 1)
			return 1;

		/*	if not found and containing directory has ended	*/
		if (ucFound == 2)
			return 2;

		/*	Otherwise, if not found but containing directory has not yet ended	*/
		/*	get number of the next cluster	*/
		uiCurrentClusterNumber = uiHOS_SDC_getNextClusterNumber(
			pxSdc,
			uiCurrentClusterNumber	);
	}

	/*	if directory clusters ended and still file has not been found	*/
	return 0;
}

/*
 * See header for info.
 */
uint32_t uiHOS_SDC_getClusterNumber(	xHOS_SDC_t* pxSdc,
										uint32_t uiFirstClusterNumber,
										uint32_t uiClusterIndex	)
{
	uint32_t uiClusterNumber = uiFirstClusterNumber;
	for (uint32_t i = 0; i < uiClusterIndex; i++)
	{
		uiClusterNumber = uiHOS_SDC_getNextClusterNumber(pxSdc, uiClusterNumber);

		if (uiClusterNumber == 0xFFFFFFFF)
			return 0xFFFFFFFF;
	}
	return uiClusterNumber;
}
