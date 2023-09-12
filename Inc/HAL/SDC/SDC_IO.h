/*
 * SDC_IO.h
 *
 *  Created on: Aug 17, 2023
 *      Author: Ali Emad
 *
 *  Notes:
 *		-	This file implements block writing and reading with/without CRC checing.
 *
 *		-	This file is private and must not be directly used in upper layers code,
 *			upper layers' writers should use: "SDC_Stream.h"
 *
 *		-	For all of this file's functions that interact with the SDC handle,
 *			handle's mutex must be first taken by the calling function, and must
 *			be released right after it's been of no need.
 *
 *		-	For all of this file's functions which use SPI (or call functions
 *			which use SPI themselves), SPI mutex must be taken first, and released
 *			as soon as SPI is no longer needed.
 */

#ifndef COTS_OS_INC_HAL_SDC_SDC_IO_H_
#define COTS_OS_INC_HAL_SDC_SDC_IO_H_


/*
 * Writes data block (512 bytes) in the SD-card.
 * Returns 1 if written successfully, 0 otherwise.
 */
uint8_t ucHOS_SDC_writeBlock(	xHOS_SDC_t* pxSdc,
								xHOS_SDC_Block_Buffer_t* pxBlock,
								TickType_t xTimeout	);

uint8_t ucHOS_SDC_keepTryingWriteBlock(	xHOS_SDC_t* pxSdc,
										xHOS_SDC_Block_Buffer_t* pxBlock,
										TickType_t xTimeout	);

/*
 * Reads data block (512 bytes) from the SD-card.
 * Returns 1 if read successfully, 0 otherwise.
 */
uint8_t ucHOS_SDC_readBlock(	xHOS_SDC_t* pxSdc,
								xHOS_SDC_Block_Buffer_t* pxBlock,
								uint32_t uiBlockNumber,
								TickType_t xTimeout	);

uint8_t ucHOS_SDC_keepTryingReadBlock(	xHOS_SDC_t* pxSdc,
										xHOS_SDC_Block_Buffer_t* pxBlock,
										uint32_t uiBlockNumber,
										TickType_t xTimeout	);












#endif /* COTS_OS_INC_HAL_SDC_SDC_IO_H_ */
