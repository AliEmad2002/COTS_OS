/*
 * SDC_Helper.h
 *
 *  Created on: Aug 16, 2023
 *      Author: Ali Emad
 *
 * Notes:
 * 		-	This file contains all private data-structures which must be part of
 * 			SDC data-structure itself.
 */

#ifndef COTS_OS_INC_HAL_SDC_SDC_HELPER_H_
#define COTS_OS_INC_HAL_SDC_SDC_HELPER_H_

#include "HAL/SDC/SDC_config.h"

/*******************************************************************************
 * SD-Card version
 ******************************************************************************/
typedef enum{
	xHOS_SDC_Version_Unknown,
	xHOS_SDC_Version_3,
	xHOS_SDC_Version_2_ByteAddress,
	xHOS_SDC_Version_2_BlockAddress,
	xHOS_SDC_Version_1
}xHOS_SDC_Version_t;

/*******************************************************************************
 * File Allocation Table
 ******************************************************************************/
typedef struct{
	uint32_t uiLba;			  	// LBA of the File Allocation Table.
	uint32_t uiSectorsPerFat;	// Number of sectors of the File Allocation Table.
}xHOS_SDC_FAT_t;

/*******************************************************************************
 * Memory buffer
 ******************************************************************************/
typedef struct{
	/*
	 * Block buffer.
	 * TODO: make this driver word accessible for faster execution.
	 */
	uint8_t pucBufferr[configHOS_SDC_BUFFER_SIZE];

	/*	LBA of the sector currently available in the buffer.	*/
	uint32_t uiLbaRead;
}xHOS_SDC_Block_Buffer_t;




#endif /* COTS_OS_INC_HAL_SDC_SDC_HELPER_H_ */
