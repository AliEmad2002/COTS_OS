/*
 * SDC_Dir.h
 *
 *  Created on: Aug 18, 2023
 *      Author: Ali Emad
 */

#ifndef COTS_OS_INC_HAL_SDC_SDC_DIR_H_
#define COTS_OS_INC_HAL_SDC_SDC_DIR_H_


/*
 * Returns directory type given directory data.
 */
SDC_DirRecordType_t xHOS_SDC_getDirRecordType(SDC_DirData_t* pxRec);

/*
 * Converts user written file name to a FAT32 understandable file name.
 * Example:
 * 		"file.nc" ==> "file    nc "
 */
void vHOS_SDC_getInFileName(char pcInFileName[11], char* pcFileName);

/*
 * Compares 11 bytes with 11 bytes
 */
uint8_t ucHOS_SDC_areEqualNames(char* pcName1, char* pcName2);

/*
 * Returns LBA of a cluster given its index.
 */
uint32_t uiHOS_SDC_getClusterLba(xHOS_SDC_t* pxSdc, uint32_t uiClusterNumber);

/*
 * Searches for a file by its name in the sector that is currently buffered.
 *
 * Returns 0 if not found but containing directory has not yet ended.
 * Returns 1 if found.
 * Returns 2 if not found and containing directory has ended.
 */
uint8_t ucHOS_SDC_findDirDataInCurrentSector(	xHOS_SDC_t* pxSdc,
												char* pcInFileName,
												SDC_DirData_t** ppxDirData	);

/*
 * Searches for a file by its name in a given cluster.
 *
 * Returns 0 if not found but containing directory has not yet ended.
 * Returns 1 if found.
 * Returns 2 if not found and containing directory has ended.
 */
uint8_t ucHOS_SDC_findDirDataInCluster(	xHOS_SDC_t* pxSdc,
										char* pcInFileName,
										uint32_t uiClusterNumber,
										SDC_DirData_t** ppxDirData	);

/*	From the FAT, this function returns index of the next cluster.	*/
uint32_t uiHOS_SDC_getNextClusterNumber(	xHOS_SDC_t* pxSdc,
											uint32_t uiCurrentClusterNumber	);

/*
 * Searches for a file by its name in a given directory.
 *
 * Returns 0 if not found.
 * Returns 1 if found.
 */
uint8_t ucHOS_SDC_findDirDataInDirectory(	xHOS_SDC_t* pxSdc,
											char* pcInFileName,
											uint32_t uiDirFirstClusterNumber,
											SDC_DirData_t** dirDataPP	);

/*	Gets cluster number of a cluster given its index, and first cluster number	*/
uint32_t uiHOS_SDC_getClusterNumber(	xHOS_SDC_t* pxSdc,
										uint32_t uiFirstClusterNumber,
										uint32_t uiClusterIndex	);



















#endif /* COTS_OS_INC_HAL_SDC_SDC_DIR_H_ */
