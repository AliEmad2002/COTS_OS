/*
 * UART.h
 *
 *  Created on: Sep 14, 2023
 *      Author: Ali Emad
 */

#ifndef COTS_OS_INC_HAL_UART_H_
#define COTS_OS_INC_HAL_UART_H_

/*******************************************************************************
 * API functions/macros
 ******************************************************************************/
/*
 * Initializes UART driver.
 *
 *		-	HW settings like frame format and clock mode must be initially set
 * 			by user.
 */
void vHOS_UART_init(void);

/*
 * Sends an array of bytes.
 *
 * Notes:
 * 		-	Before calling this function, there must not be any on-going transfer
 * 			operations on the same unit, otherwise previous transmission would
 * 			be omitted.
 *
 *		-	As UART is asynchronous, it is recommended to not send too much bytes
 *			consecutively, so that transmitter and receiver don't get out of sync.
 *
 * 		-	After calling this function, tasks can synchronize end of send
 * 			operation using the function "xHOS_UART_blockUntilTransferComplete()".
 */
void vHOS_UART_send(uint8_t ucUnitNumber, int8_t* pcArr, uint32_t uiSize);

/*
 * Receives an array of bytes.
 *
 * Notes:
 * 		-	If the specified "uiSize" was not received within the "xTimeout",
 * 			function returns 0. Otherwise, it returns 1.
 */
uint8_t ucHOS_UART_receive(		uint8_t ucUnitNumber,
								int8_t* pcInArr,
								uint32_t uiSize,
								TickType_t xTimeout	);

/*
 * Acquires unit's mutex for the calling task.
 *
 * Notes:
 * 		-	Can only be used after scheduler start.
 *
 * 		-	Can only be called from inside a task.
 *
 * 		-	Unit's mutex must be taken (this function must be called) first
 * 			before using any of the above functions.
 *
 * 		-	This is an inline function.
 */
uint8_t ucHOS_UART_takeMutex(uint8_t ucUnitNumber, TickType_t xTimeout);

/*
 * Releases unit's mutex from the calling task.
 *
 * Notes:
 * 		-	Can only be used after scheduler start.
 *
 * 		-	Can only be called from inside a task which has previously taken
 * 			this mutex, and is currently holding it.
 *
 * 		-	This is an inline function.
 */
void vHOS_UART_releaseMutex(uint8_t ucUnitNumber);

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
 *		-	As UART is asynchronous, it is recommended to delay (for an experimental
 *			amount of time that depends on transmission delay) after each "send"
 *			operation.
 *
 * 		-	This is an inline function.
 */
uint8_t ucHOS_UART_blockUntilTransferComplete(uint8_t ucUnitNumber, TickType_t xTimeout);



#endif /* COTS_OS_INC_HAL_UART_H_ */
