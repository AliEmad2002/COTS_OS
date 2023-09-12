/*
 * Port_USB.h
 *
 *  Created on: Sep 11, 2023
 *      Author: Ali Emad
 *
 * This driver - so far - implements only the virtual COM port functionality.
 */

#ifndef COTS_OS_INC_MCAL_PORT_PORT_USB_H_
#define COTS_OS_INC_MCAL_PORT_PORT_USB_H_

#include "stm32f1xx.h"
#include "stm32f1xx_hal.h"
#include "usbd_def.h"
#include "usbd_core.h"
#include "usbd_cdc.h"
#include "usb_device.h"
#include "usbd_desc.h"
#include "usbd_cdc_if.h"

#include "LIB/Assert.h"

extern USBD_HandleTypeDef hUsbDeviceFS;

/*
 * Rx buffer info:
 */
extern uint8_t* pucPortUsbRxBuffer;
extern uint32_t* puiPortUsbRxBufferLen;





static inline void vPort_USB_initHardware(void)
{
	vLib_ASSERT(USBD_Init(&hUsbDeviceFS, &FS_Desc, DEVICE_FS) == USBD_OK, 0);
	vLib_ASSERT(USBD_RegisterClass(&hUsbDeviceFS, &USBD_CDC) == USBD_OK, 0);
	vLib_ASSERT(USBD_CDC_RegisterInterface(&hUsbDeviceFS, &USBD_Interface_fops_FS) == USBD_OK, 0);
	vLib_ASSERT(USBD_Start(&hUsbDeviceFS) == USBD_OK, 0);
}

static inline uint8_t ucPort_USB_send(uint8_t* pucBuffer, uint16_t usLen)
{
	return (CDC_Transmit_FS(pucBuffer, usLen) == USBD_OK);
}




#endif /* COTS_OS_INC_MCAL_PORT_PORT_USB_H_ */
