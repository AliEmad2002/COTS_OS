/*
 * Port_USB.c
 *
 *  Created on: Sep 11, 2023
 *      Author: Ali Emad
 */

/*	LIB	*/
#include "LIB/Assert.h"

/*	MCAL	*/
#include "MCAL_Port/Port_USB.h"
#include "MCAL_Port/Port_Interrupt.h"

/*	OS	*/
#include "FreeRTOSConfig.h"

/*	Dependencies	*/
#include "tusb.h"

/*******************************************************************************
 * Static (Private) variables:
 ******************************************************************************/
#define USBD_STACK_SIZE    (3*configMINIMAL_STACK_SIZE)
static StackType_t  pxUsbDeviceTaskStack[USBD_STACK_SIZE];
static StaticTask_t xUsbDeviceTaskStatic;
static TaskHandle_t xUsbDeviceTask;


/*******************************************************************************
 * Static (Private) functions:
 ******************************************************************************/
/*
 * USB device class RTOS task. Copied from TinyUSB example.
 */
static void vUsbDeviceTask(void *param)
{
  (void) param;

  // init device stack on configured roothub port
  // This should be called after scheduler/kernel is started.
  // Otherwise it could cause kernel issue since USB IRQ handler does use RTOS queue API.
  tud_init(BOARD_TUD_RHPORT);

  // RTOS forever loop
  while (1)
  {
    // put this thread to waiting state until there is new events
    tud_task();

    /*
     * send whatever data is temporarily kept in the buffers. (this operation is
     * done  only if "flush()" was not performed after every write operation.
     */
    //tud_cdc_n_write_flush(0);
  }
}



/*******************************************************************************
 * API functions:
 ******************************************************************************/
/*
 * See header for info.
 */
void vPort_USB_initHardware(void)
{
	/*	Set USB interrupts' priorities	*/
	VPORT_INTERRUPT_SET_PRIORITY(xPortInterruptUsbHighPriIrqNumber, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
	VPORT_INTERRUPT_SET_PRIORITY(xPortInterruptUsbLowPriIrqNumber, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
	VPORT_INTERRUPT_SET_PRIORITY(xPortInterruptUsbWkupIrqNumber, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);

	/*	Configure USB DM (data minus) and DP (data plus) pins	*/
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = (GPIO_PIN_11 | GPIO_PIN_12);
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*	Initialize USB tasks	*/
	xUsbDeviceTask = xTaskCreateStatic(
						vUsbDeviceTask,
						"usbd",
						USBD_STACK_SIZE,
						NULL,
						configMAX_PRIORITIES - 1,
						pxUsbDeviceTaskStack,
						&xUsbDeviceTaskStatic	);
}

#ifdef ucPORT_INTERRUPT_IRQ_DEF_USB
//--------------------------------------------------------------------+
// Forward USB interrupt events to TinyUSB IRQ Handler
//--------------------------------------------------------------------+
void USB_HP_IRQHandler(void) {
  tud_int_handler(0);
}

void USB_LP_IRQHandler(void) {
  tud_int_handler(0);
}

void USBWakeUp_IRQHandler(void) {
  tud_int_handler(0);
}
#endif
