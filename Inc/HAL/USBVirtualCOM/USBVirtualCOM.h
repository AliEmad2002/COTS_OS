/*
 * USBVirtualCOM.h
 *
 *  Created on: Sep 11, 2023
 *      Author: Ali Emad
 */

#ifndef COTS_OS_INC_HAL_USBVIRTUALCOM_USB_VIRTUALCOM_H_
#define COTS_OS_INC_HAL_USBVIRTUALCOM_USB_VIRTUALCOM_H_


/*
 * Initializes USB virtual COM port.
 */
void vHOS_USBVirtualCOM_init(void);

/*
 * Locks USB driver.
 *
 * Notes:
 * 		-	This function must be called before performing any USB operations.
 *
 * 		-	This function waits for the USB driver to be available for a timeout
 * 			of "xTimeout".
 *
 * 		-	If successfully locked, this function returns 1, otherwise it returns 0.
 */
uint8_t ucHOS_USBVirtualCOM_lockDriver(TickType_t xTimeout);

/*
 * Releases USB driver.
 *
 * Notes:
 * 		-	This function must be called after the locking task is done using the
 * 			USB driver.
 */
void vHOS_USBVirtualCOM_releaseDriver(void);

/*
 * Blocks until the ongoing send operation is done.
 */
void vHOS_USBVirtualCOM_blockUntilTxDone(void);

/*
 * Stars a send operation.
 */
void vHOS_USBVirtualCOM_send(uint8_t* pucBuffer, uint16_t usLen);

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
uint8_t ucHOS_USBVirtualCOM_readRxBuffer(	uint8_t* pucBuffer,
											uint32_t* puiLen,
											TickType_t xTimeout	);




#endif /* COTS_OS_INC_HAL_USBVIRTUALCOM_USB_VIRTUALCOM_H_ */
