/*
 * DMA.h
 *
 *  Created on: Aug 23, 2023
 *      Author: Ali Emad
 */

#ifndef COTS_OS_INC_HAL_DMA_DMA_H_
#define COTS_OS_INC_HAL_DMA_DMA_H_

#include "MCAL_Port/Port_DMA.h"

typedef xPort_DMA_TransInfo_t xHOS_DMA_TransInfo_t;

/*
 * Initializes driver.
 *
 * Notes:
 * 		-	Must be called before scheduler start.
 */
void vHOS_DMA_init(void);

/*
 * Locks mutex of the first available channel in the DMA channels
 * pool.
 *
 * Notes:
 * 		-	Returns 1 if successfully locked a channel, 0 if timeout was reached.
 *
 * 		-	If channel is locked successfully, unit number and channel number are
 * 			written to "pucUnitNumber" and "pucChannelNumber".
 *
 * 		-	After done using the locked channel, it must be released using the
 * 			function: "vHOS_DMA_releaseChannel()"
 *
 * 		-	This function is ideal for memory to memory operations, as it
 * 			dynamically locks the first available channel, resulting in less
 * 			waiting time.
 */
uint8_t ucHOS_DMA_lockAnyChannel(	uint8_t* pucUnitNumber,
									uint8_t* pucChannelNumber,
									TickType_t xTimeout	);


/*
 * Locks mutex of a certain given channel.
 *
 * Notes:
 * 		-	Returns 1 if successfully locked a channel, 0 if timeout was reached.
 *
 * 		-	This method is less efficient than the previous, but it is mandatory
 * 			in some cases.
 *
 * 		-	After done using the locked channel, it must be released using the
 * 			function: "vHOS_DMA_releaseChannel()"
 *
 * 		-	This function is used for channel dependent operations, such as
 * 			peripheral DMA operations in some targets, which has static mapping
 * 			between certain channels and certain peripherals.
 *
 * 		-	When implementing drivers using DMA, it is recommended to make two
 * 			implementations, one for targets which have static mapping (using
 * 			this function), and another for targets which have dynamic mapping
 * 			(using previous function).
 */
uint8_t ucHOS_DMA_lockChannel(	uint8_t ucUnitNumber,
								uint8_t ucChannelNumber,
								TickType_t xTimeout	);

/*
 * Initiates a DMA transfer.
 *
 * Notes:
 * 		-	The used channel must be previously locked by the calling task.
 */
void vHOS_DMA_startTransfer(xHOS_DMA_TransInfo_t* pxInfo);

/*
 * Blocks the calling task until transfer is completed.
 *
 * Notes:
 * 		-	Returns 1 if transfer completed, 0 if timeout was reached.
 */
uint8_t ucHOS_DMA_blockUntilTransferComplete(	uint8_t ucUnitNumber,
												uint8_t ucChannelNumber,
												TickType_t xTimeout	);

/*
 * Clears transfer complete flag.
 *
 * Notes:
 * 		-	TC flags must be cleared before starting a transfer.
 */
void vHOS_DMA_clearTransferCompleteFlag(	uint8_t ucUnitNumber,
											uint8_t ucChannelNumber	);

/*
 * Releases a previously locked channel.
 *
 * Notes:
 * 		-	Returns 1 if successfully released a channel, 0 if timeout was reached.
 *
 * 		-	Must be called in the same task which has locked the channel. (As
 * 			resource synchronization in this driver uses mutex, which follows
 * 			principle of ownership)
 *
 * 		-	If timed-out, task must never ignore releasing the channel, and must
 * 			try again.
 */
uint8_t ucHOS_DMA_releaseChannel(	uint8_t ucUnitNumber,
								uint8_t ucChannelNumber,
								TickType_t xTimeout	);


#endif /* COTS_OS_INC_HAL_DMA_DMA_H_ */
