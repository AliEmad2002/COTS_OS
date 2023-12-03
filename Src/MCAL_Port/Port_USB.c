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
#if CFG_TUD_MSC
#define USBD_STACK_SIZE    (3*configMINIMAL_STACK_SIZE)
static StackType_t  usb_device_stack[USBD_STACK_SIZE];
static StaticTask_t usb_device_taskdef;
#endif

#if CFG_TUD_CDC
#define CDC_STACK_SZIE      configMINIMAL_STACK_SIZE
static StackType_t  cdc_stack[CDC_STACK_SZIE];
static StaticTask_t cdc_taskdef;
#endif


/*******************************************************************************
 * Static (Private) functions:
 ******************************************************************************/
/*
 * USB device class RTOS task. Copied from TinyUSB example.
 */
static void usb_device_task(void *param)
{
  (void) param;

  // init device stack on configured roothub port
  // This should be called after scheduler/kernel is started.
  // Otherwise it could cause kernel issue since USB IRQ handler does use RTOS queue API.
  tud_init(BOARD_TUD_RHPORT);

  // RTOS forever loop
  while (1) {
    // put this thread to waiting state until there is new events
    tud_task();

    // following code only run if tud_task() process at least 1 event
#if CFG_TUD_CDC
    tud_cdc_write_flush();
#endif
  }
}

/*
 * USB communication device class RTOS task. Copied from TinyUSB example.
 */
#if CFG_TUD_CDC
void cdc_task(void *params)
{
  (void) params;

  // RTOS forever loop
  while (1) {
    // connected() check for DTR bit
    // Most but not all terminal client set this when making connection
    // if ( tud_cdc_connected() )
    {
      // There are data available
      while (tud_cdc_available()) {
        uint8_t buf[64];

        // read and echo back	/*	TODO: change this and link it with the COTS_OS CDC driver	*/
        uint32_t count = tud_cdc_read(buf, sizeof(buf));
        (void) count;

        // Echo back
        // Note: Skip echo by commenting out write() and write_flush()
        // for throughput test e.g
        //    $ dd if=/dev/zero of=/dev/ttyACM0 count=10000
        tud_cdc_write(buf, count);
      }

      tud_cdc_write_flush();
    }
  }
}
#endif


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
#if CFG_TUD_MSC
	xTaskCreateStatic(	usb_device_task,
						"usbd",
						USBD_STACK_SIZE,
						NULL,
						configMAX_PRIORITIES-1,
						usb_device_stack,
						&usb_device_taskdef	);
#endif

#if CFG_TUD_CDC
	xTaskCreateStatic(	cdc_task,
						"cdc",
						CDC_STACK_SZIE,
						NULL,
						configMAX_PRIORITIES-2,
						cdc_stack,
						&cdc_taskdef	);
#endif
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
