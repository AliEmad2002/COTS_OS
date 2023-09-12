/*
 * SDC_Stream.h
 *
 *  Created on: Aug 16, 2023
 *      Author: Ali Emad
 */

#ifndef COTS_OS_INC_HAL_SDC_SDC_STREAM_H_
#define COTS_OS_INC_HAL_SDC_SDC_STREAM_H_

#include "HAL/SDC/SDC_Helper.h"
#include "HAL/SDC/SDC.h"

/*******************************************************************************
 * Stream
 ******************************************************************************/
typedef struct{
	/*		PRIVATE		*/
	/*	LBA of the first sector in file.	*/
	uint32_t uiStartLba;

	/*	First cluster number	*/
	uint32_t uiFirstClusterNumber;

	/*	Stream's buffer	*/
	xHOS_SDC_Block_Buffer_t xBuffer;

	/*		PUBLIC		*/
	/*	pointer to the SDC handle on which this stream is located. (set only once)	*/
	xHOS_SDC_t* pxSdc;

	/*	Size (in bytes) of the allocated file on the partition (Read-only)	*/
	uint32_t uiSizeOnSDC;

	/*	Size (in bytes) of the actual opened file on the partition (Read-only)	*/
	uint32_t uiSizeActual;

	/*	Start index (in bytes) of the first un-read line. (Read-only)	*/
	uint32_t uiReader;

	/*
	 * Start index (in bytes) of the last read line.
	 * (Read-only, used in going backwards, assumes lines are separated by '\n')
	 */
	uint32_t uiLastReader;
}xHOS_SDC_Stream_t;

/*******************************************************************************
 * API functions:
 ******************************************************************************/
/**
 * Following, all functions named starting by "keepTrying" would do the same functionality
 * of their previous ones, except that these function would do the following loop continuously
 * until timeout is reached:
 * 		-	Try the function with a maximum of three chances of failure.
 * 		-	If the function still fails three times, reset the SD-card.
 * 		-	If the required functionality is done successfully, it returns.
 **/

/*
 * Opens file from the SD card on a stream object.
 * Returns 1 if successfully opened. 0 otherwise.
 *
 * Notes:
 * 		-	A previously initialized SDC handle must be assigned to the pointer
 * 			"pxSdc" in stream's handle.
 *
 * TODO:
 * 		-	So-far, this driver can only access root directory files. Make it
 * 			access files from any directory.
 * 		-	So far, this driver can only open and modify already existing files.
 * 		-	So far, this driver can not allocate new clusters to the opened file,
 * 			so make sure it initially has enough allocated size.
 */
uint8_t ucHOS_SDC_openStream(	xHOS_SDC_Stream_t* pxStream,
								char* pcFileName,
								TickType_t xTimeout	);

uint8_t ucHOS_SDC_keepTryingOpenStream(	xHOS_SDC_Stream_t* pxStream,
										char* pcFileName,
										TickType_t xTimeout	);

/*	Saves sector that is currently in buffer into the SD-card	*/
uint8_t ucHOS_SDC_saveCurrentBuffer(	xHOS_SDC_Stream_t* pxStream,
										TickType_t xTimeout	);

/*	Reads sector from SD-card to stream's buffer	*/
uint8_t ucHOS_SDC_readSector(	xHOS_SDC_Stream_t* pxStream,
								uint32_t uiSectorsOffset,
								TickType_t xTimeout	);

/*
 * Updates stream's buffer.
 *
 * If the requested byte offset is outside stream's buffer, the buffer is saved
 * to the SD-card, and the sector containing the requested byte offset is copied
 * to stream's buffer.
 *
 * Notes:
 * 		-	"uiOffset": Offset in bytes.
 */
uint8_t ucHOS_SDC_updateBbuffer(	xHOS_SDC_Stream_t* pxStream,
									uint32_t uiOffset,
									TickType_t xTimeout	);

uint8_t ucHOS_SDC_keepTryingUpdateBuffer(	xHOS_SDC_Stream_t* pxStream,
											uint32_t uiOffset,
											TickType_t xTimeout	);

/*
 * reads array of bytes from stream object.
 *
 * Notes:
 * 		-	"uiOffset" and "uiLen" are in bytes.
 */
uint8_t ucHOS_SDC_readStream(	xHOS_SDC_Stream_t* pxStream,
								uint32_t uiOffset,
								uint8_t* pucArr,
								uint32_t uiLen,
								TickType_t xTimeout	);

uint8_t ucHOS_SDC_keepTryingReadStream(	xHOS_SDC_Stream_t* pxStream,
										uint32_t uiOffset,
										uint8_t* pucArr,
										uint32_t uiLen,
										TickType_t xTimeout	);

/*
 * writes array of bytes to stream object.
 * Notes:
 * 		-	"uiOffset" and "uiLen" are in bytes.
 */
uint8_t ucHOS_SDC_writeStream(	xHOS_SDC_Stream_t* pxStream,
								uint32_t uiOffset,
								uint8_t* pucArr,
								uint32_t uiLen,
								TickType_t xTimeout	);

uint8_t ucHOS_SDC_keepTryingWriteStream(	xHOS_SDC_Stream_t* pxStream,
											uint32_t uiOffset,
											uint8_t* pucArr,
											uint32_t uiLen,
											TickType_t xTimeout	);

/*	Closes / Saves stream	*/
uint8_t ucHOS_SDC_saveStream(xHOS_SDC_Stream_t* pxStream, TickType_t xTimeout);

uint8_t ucHOS_SDC_keepTryingSaveStream(	xHOS_SDC_Stream_t* pxStream,
										TickType_t xTimeout	);

/*	Reads next un-read line of an opened text (ASCII formatted) file	*/
uint8_t ucHOS_SDC_getNextLine(	xHOS_SDC_Stream_t* pxStream,
								char* pcLine,
								uint32_t uiMaxSize,
								TickType_t xTimeout	);

/*	Resets line reader offset	*/
void vHOS_SDC_resetLineReader(xHOS_SDC_Stream_t* pxStream);

/*	Checks if there's a next line	*/
uint8_t ucHOS_SDC_isThereNextLine(xHOS_SDC_Stream_t* pxStream);

/*
 * Sets "stream->reader"'s value to the start of previous line.
 * Notice it can seek only once in a row, meaning it can not seek further than
 * just the previous line if called multiple times. As the stream handle only
 * stores start of current and previous lines.
 */
uint8_t ucHOS_SDC_seekReaderPrevLine(xHOS_SDC_Stream_t* pxStream);













#endif /* COTS_OS_INC_HAL_SDC_SDC_STREAM_H_ */
