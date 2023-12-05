/*
 * UsbCdc.h
 *
 *  Created on: Sep 11, 2023
 *      Author: Ali Emad
 */

#ifndef COTS_OS_INC_HAL_USB_CDC_USB_CDC_H_
#define COTS_OS_INC_HAL_USB_CDC_USB_CDC_H_

#include "FreeRTOS.h"


/*
 * Initializes USB CDC driver.
 */
void vHOS_UsbCdc_init(void);

/*
 * Locks USB transmission for the caller task.
 *
 * Notes:
 * 		-	This function must be called before performing any USB transmit.
 *
 * 		-	This function waits for the USB transmission to be available for a timeout
 * 			of "xTimeout".
 *
 * 		-	If successfully locked, this function returns 1, otherwise it returns 0.
 */
uint8_t ucHOS_UsbCdc_lockTransmission(uint8_t ucUnitNumber, TickType_t xTimeout);

/*
 * Releases USB transmission.
 *
 * Notes:
 * 		-	This function must be called from withing the same task which have locked
 * 			it (Mutex based method).
 */
void vHOS_UsbCdc_releaseTransmission(uint8_t ucUnitNumber);

/*
 * Locks USB reception for the caller task.
 *
 * Notes:
 * 		-	This function must be called before performing any USB reception.
 *
 * 		-	This function waits for the USB reception to be available for a timeout
 * 			of "xTimeout".
 *
 * 		-	If successfully locked, this function returns 1, otherwise it returns 0.
 */
uint8_t ucHOS_UsbCdc_lockReception(uint8_t ucUnitNumber, TickType_t xTimeout);

/*
 * Releases USB reception.
 *
 * Notes:
 * 		-	This function must be called from withing the same task which have locked
 * 			it (Mutex based method).
 */
void vHOS_UsbCdc_releaseReception(uint8_t ucUnitNumber);

/*
 * Blocks until the ongoing send operation is done.
 */
void vHOS_UsbCdc_blockUntilTxDone(uint8_t ucUnitNumber);

/*
 * Stars a send operation.
 */
void vHOS_UsbCdc_send(uint8_t ucUnitNumber, uint8_t* pucBuffer, uint16_t usLen);

/*
 * Reads the Rx buffer.
 *
 * Notes:
 * 		-	This function copies Rx buffer to "pucBuffer", and length of the
 * 			buffer to "*pusLen".
 *
 * 		-	If Rx buffer was empty, this function waits for "xTimeout". If timeout
 * 			ends and no data is received, function returns 0. Otherwise it
 * 			returns 1.
 *
 */
uint8_t ucHOS_UsbCdc_readRxBuffer(	uint8_t ucUnitNumber,
									uint8_t* pucBuffer,
									uint32_t* puiLen,
									TickType_t xTimeout	);

/*
 * Receives N bytes with a a timeout.
 *
 * Notes:
 * 		-	If N bytes were not received in the given timeout, function returns 0.
 * 			Otherwise, it returns 1.
 */
uint8_t ucHOS_UsbCdc_receive(	uint8_t ucUnitNumber,
								uint8_t* pucBuffer,
								uint32_t uiLen,
								TickType_t xTimeout	);



#endif /* COTS_OS_INC_HAL_USB_CDC_USB_CDC_H_ */
