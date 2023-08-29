/*
 * SPI.h
 *
 *  Created on: Jun 12, 2023
 *      Author: Ali Emad
 *
 */

#ifndef HAL_OS_INC_SPI_SPI_H_
#define HAL_OS_INC_SPI_SPI_H_

/*******************************************************************************
 * Include dependencies:
 ******************************************************************************/
#include "HAL/SPI/SPIConfig.h"
#include "semphr.h"

/*******************************************************************************
 * API functions/macros
 ******************************************************************************/
#define ucHOS_SPI_BYTE_DIRECTION_MSBYTE_FIRST		0
#define ucHOS_SPI_BYTE_DIRECTION_LSBYTE_FIRST		1

/*
 * Initializes SPI driver.
 *
 *		-	HW settings like frame format and clock mode must be initially set
 * 			by user.
 */
void vHOS_SPI_init(void);

/*
 * Sets byte sending direction.
 *
 * Notes:
 * 		-	"ucByteDirection": must be one of:
 * 			-	"ucHOS_SPI_BYTE_DIRECTION_MSBYTE_FIRST".
 * 			-	"ucHOS_SPI_BYTE_DIRECTION_LSBYTE_FIRST".
 *
 * 		-	Byte direction is not same as bit direction, the later is configured
 * 			when initializing the SPI HW module.
 *
 * 		-	Byte direction must not be changed while a transfer operation is running,
 * 			otherwise this operation won't complete successfully.
 */
void vHOS_SPI_setByteDirection(uint8_t ucUnitNumber, uint8_t ucByteDirection);

/*
 * Sends an array of bytes. (Ignores received bytes)
 *
 * Notes:
 * 		-	Before calling this function, there must not be any on-going transfer
 * 			operations on the same SPI unit, otherwise previous transmission would
 * 			be omitted.
 *
 * 		-	After calling this function, tasks can synchronize end of send
 * 			operation using the function "xHOS_SPI_blockUntilTransferComplete()".
 */
void vHOS_SPI_send(uint8_t ucUnitNumber, int8_t* pcArr, uint32_t uiSize);

/*
 * Transceives an array of bytes.
 */
void vHOS_SPI_transceive(	uint8_t ucUnitNumber,
							int8_t* pcOutArr,
							int8_t* pcInArr,
							uint32_t uiSize	);

/*
 * Receives an array of bytes. (Sends 0xFF)
 */
void vHOS_SPI_receive(		uint8_t ucUnitNumber,
							int8_t* pcInArr,
							uint32_t uiSize	);

/*	sends same array multiple times	*/
void vHOS_SPI_sendMultiple(	uint8_t ucUnitNumber,
							int8_t* pcArr,
							uint32_t uiSize,
							uint32_t uiN	);

/*
 * Acquires SPI unit's mutex for the calling task.
 *
 * Notes:
 * 		-	Can only be used after scheduler start.
 *
 * 		-	Can only be called from inside a task.
 *
 * 		-	SPI unit's mutex must be taken (this function must be called) first
 * 			before using any of the above functions.
 *
 * 		-	This is an inline function.
 */
uint8_t ucHOS_SPI_takeMutex(uint8_t ucUnitNumber, TickType_t xTimeout);

/*
 * Releases SPI unit's mutex from the calling task.
 *
 * Notes:
 * 		-	Can only be used after scheduler start.
 *
 * 		-	Can only be called from inside a task which has previously taken
 * 			this mutex, and is currently holding it.
 *
 * 		-	This is an inline function.
 */
void vHOS_SPI_releaseMutex(uint8_t ucUnitNumber);

/*
 * Blocks calling task until transfer is complete or timeout passes.
 *
 * Notes:
 * 		-	Returns 1 if transfer completed while timeout hasn't passed, otherwise
 * 			returns 0.
 *
 * 		-	Can't be used twice in a row. It's like a flag that gets set when
 * 			transfer actually is complete, and cleared when this function is called.
 *
 * 		-	This is an inline function.
 */
uint8_t ucHOS_SPI_blockUntilTransferComplete(uint8_t ucUnitNumber, TickType_t xTimeout);







#endif /* HAL_OS_INC_SPI_SPI_H_ */
