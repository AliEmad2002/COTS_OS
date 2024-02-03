/*
 * Port_USB.c
 *
 *  Created on: Sep 11, 2023
 *      Author: Ali Emad
 */

/*	Target checking	*/
#include "MCAL_Port/Port_Target.h"
#ifdef MCAL_PORT_TARGET_STM32F401RCT6


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
#define USBD_STACK_SIZE    (700/4)
static StackType_t  pxUsbDeviceTaskStack[USBD_STACK_SIZE];
static StaticTask_t xUsbDeviceTaskStatic;
static TaskHandle_t xUsbDeviceTask;

/*******************************************************************************
 * Global variables:
 ******************************************************************************/
PCD_HandleTypeDef hpcd_USB_OTG_FS;

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
	  hpcd_USB_OTG_FS.Instance = USB_OTG_FS;
	  hpcd_USB_OTG_FS.Init.dev_endpoints = 4;
	  hpcd_USB_OTG_FS.Init.speed = PCD_SPEED_FULL;
	  hpcd_USB_OTG_FS.Init.dma_enable = DISABLE;
	  hpcd_USB_OTG_FS.Init.phy_itface = PCD_PHY_EMBEDDED;
	  hpcd_USB_OTG_FS.Init.Sof_enable = DISABLE;
	  hpcd_USB_OTG_FS.Init.low_power_enable = DISABLE;
	  hpcd_USB_OTG_FS.Init.lpm_enable = DISABLE;
	  hpcd_USB_OTG_FS.Init.vbus_sensing_enable = DISABLE;
	  hpcd_USB_OTG_FS.Init.use_dedicated_ep1 = DISABLE;
	  vLib_ASSERT(HAL_PCD_Init(&hpcd_USB_OTG_FS) == HAL_OK, 0);

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
void OTG_FS_IRQHandler(void) {
  tud_int_handler(0);
}

void OTG_FS_WKUP_IRQHandler(void) {
  tud_int_handler(0);
}
#endif


#endif /* Target checking */
