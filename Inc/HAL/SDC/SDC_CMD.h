/*
 * SDC_CMD.h
 *
 *  Created on: Aug 16, 2023
 *      Author: Ali Emad
 *
 * Notes:
 *		-	This file implements command sending and command response receiving.
 *
 *		-	This file is private and must not be directly used in upper layers code,
 *			upper layers' writers should use: "SDC_Stream.h"
 *
 *		-	For all of this file's functions that interact with the SDC handle,
 *			handle's mutex must be first taken by the calling function, and must
 *			be released right after it's been of no need.
 */

#ifndef COTS_OS_INC_HAL_SDC_SDC_CMD_H_
#define COTS_OS_INC_HAL_SDC_SDC_CMD_H_



/*******************************************************************************
 * Sends command to the SDC.
 *
 * Notes:
 * 		-	"pxSdc": must be a valid and initialized handle.
 * 		-	"ucIndex": command's index.
 * 		-	"uiArg": command's argument.
 ******************************************************************************/
void vHOS_SDC_sendCommand(xHOS_SDC_t* pxSdc, uint8_t ucIndex, uint32_t uiArg);

/*******************************************************************************
 * Gets data response. (Received after sending data block)
 ******************************************************************************/
uint8_t ucHOS_SDC_getDataResponse(	xHOS_SDC_t* pxSdc,
									SDC_Data_Response_t* pxResponse	);

/*******************************************************************************
 * Receives r1 response with a time out of 8 SPI bytes. as:
 * "The response is sent back within command response time (NCR), 0 to 8 bytes
 * for SDC, 1 to 8 bytes for MMC".
 *
 * Returns 1 if received, 0 otherwise.
 ******************************************************************************/
uint8_t ucHOS_SDC_getR1(xHOS_SDC_t* pxSdc, SDC_R1_t* pxResponse);

/*******************************************************************************
 * Checks a previously received r1 response. If any of the error flags was raised,
 * it returns 0, otherwise returns 1.
 ******************************************************************************/
uint8_t ucHOS_SDC_checkR1(SDC_R1_t* pxResponse);

/*******************************************************************************
 * Receives r3 response.
 ******************************************************************************/
uint8_t ucHOS_SDC_getR3(xHOS_SDC_t* pxSdc, SDC_R3_t* pxResponse);

/*******************************************************************************
 * Receives r7 response.
 ******************************************************************************/
uint8_t ucHOS_SDC_getR7(xHOS_SDC_t* pxSdc, SDC_R7_t* pxResponse);

/*******************************************************************************
 * Sends a command of the type that must be sent in sequence.
 ******************************************************************************/
uint8_t ucHOS_SDC_sendAcmd(xHOS_SDC_t* pxSdc, uint8_t ucIndex, uint32_t uiArg);

/*******************************************************************************
 * Acquires OCR value from the SDC.
 ******************************************************************************/
uint8_t ucHOS_SDC_getOcr(xHOS_SDC_t* pxSdc, SDC_OCR_t* pxOcr);

/*******************************************************************************
 * Sets block length.
 *
 * Notes:
 * 		-	"uiLen" is length in bytes.
 ******************************************************************************/
uint8_t ucHOS_SDC_setBlockLen(xHOS_SDC_t* pxSdc, uint32_t uiLen);

/*******************************************************************************
 * Enables / Disables CRC for reading / writing blocks.
 ******************************************************************************/
uint8_t ucHOS_SDC_writeCrcEnable(xHOS_SDC_t* pxSdc, uint8_t ucCrcEnable);





















#endif /* COTS_OS_INC_HAL_SDC_SDC_CMD_H_ */
