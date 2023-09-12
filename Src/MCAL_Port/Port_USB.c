/*
 * Port_USB.c
 *
 *  Created on: Sep 11, 2023
 *      Author: Ali Emad
 */

#include "MCAL_Port/Port_USB.h"
#include "MCAL_Port/Port_Interrupt.h"



/*
 * Rx buffer info:
 */
extern uint8_t UserRxBufferFS[APP_RX_DATA_SIZE];
uint8_t* pucPortUsbRxBuffer = UserRxBufferFS;

extern uint32_t UserRxBufferFSLen;
uint32_t* puiPortUsbRxBufferLen = &UserRxBufferFSLen;

/*
 * IRQ handler:
 */
#ifdef ucPORT_INTERRUPT_IRQ_DEF_USB
void USB_LP_CAN1_RX0_IRQHandler(void)
{
	extern PCD_HandleTypeDef hpcd_USB_FS;
	HAL_PCD_IRQHandler(&hpcd_USB_FS);
}
#endif

