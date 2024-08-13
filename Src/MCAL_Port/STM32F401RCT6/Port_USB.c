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
#include "usb_host.h"
#include "usbh_cdc.h"

#ifdef ucPORT_USB_ENABLE

/*******************************************************************************
 * Static (Private) variables:
 ******************************************************************************/
#define USB_STACK_SIZE    (700/4)
static StackType_t  pxUsbTaskStack[USB_STACK_SIZE];
static StaticTask_t xUsbTaskStatic;
TaskHandle_t xUsbTask;

/*******************************************************************************
 * Global variables:
 ******************************************************************************/
PCD_HandleTypeDef hpcd_USB_OTG_FS;

/*******************************************************************************
 * Static (Private) functions:
 ******************************************************************************/
#ifdef ucPORT_USB_RUN_IN_DEVICE_MODE
static void vInit(void)
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
}
#endif	/*	ucPORT_USB_RUN_IN_DEVICE_MODE	*/

#ifdef ucPORT_USB_RUN_IN_HOST_MODE
static void vInit(void)
{
	/* Init host Library, add supported class and start the library. */
	MX_USB_HOST_Init();

	/*	Set USB interrupts' priorities	*/
	VPORT_INTERRUPT_SET_PRIORITY(xPortInterruptUsbHighPriIrqNumber, 0);
	VPORT_INTERRUPT_SET_PRIORITY(xPortInterruptUsbLowPriIrqNumber, 0);
	VPORT_INTERRUPT_SET_PRIORITY(xPortInterruptUsbWkupIrqNumber, 0);
}

extern USBH_HandleTypeDef hUsbHostFS;
extern ApplicationTypeDef Appli_state;
USBH_StatusTypeDef usbresult;

#define RX_BUFF_SIZE   64  /* Max Received data 1KB */

uint8_t CDC_RX_Buffer[RX_BUFF_SIZE];
uint8_t CDC_TX_Buffer[RX_BUFF_SIZE];

typedef enum {
  CDC_STATE_IDLE = 0,
  CDC_RECEIVE,
}CDC_StateTypedef;

CDC_StateTypedef CDC_STATE = CDC_STATE_IDLE;

uint8_t i=0;
uint8_t j=0;
uint32_t delay = 100;
uint8_t txCplt = 1;
uint8_t rxCplt = 1;
void CDC_HANDLE (void)
{
	switch (CDC_STATE)
	{
	case CDC_STATE_IDLE:
	{
		while(txCplt == 0);
		vPortEnterCritical();
		txCplt = 0;
//		USBH_CDC_Stop(&hUsbHostFS);
		if (USBH_CDC_Transmit (&hUsbHostFS, &i, 1) == USBH_OK)
		  {
			  CDC_STATE = CDC_RECEIVE;
			  vPortExitCritical();
			  HAL_Delay (pdMS_TO_TICKS(delay));
		  }
		  i+=2;
		  break;
	}

	case CDC_RECEIVE:
	{
			while(rxCplt == 0);
			vPortEnterCritical();
			USBH_CDC_Stop(&hUsbHostFS);
			rxCplt = 0;
		  usbresult = USBH_CDC_Receive(&hUsbHostFS, &j, 1);
	  vPortExitCritical();
//		  while(!rxCplt);
//		  rxCplt = 0;
	  HAL_Delay (pdMS_TO_TICKS(delay));
		  CDC_STATE = CDC_IDLE;
	}

	default:
		  break;
	}
}
#endif	/*	ucPORT_USB_RUN_IN_HOST_MODE	*/


/*******************************************************************************
 * RTOS task:
 ******************************************************************************/
#ifdef ucPORT_USB_RUN_IN_DEVICE_MODE
/*
 * USB device class RTOS task. Copied from TinyUSB example.
 */
static void vUsbTask(void *param)
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
#endif	/*	ucPORT_USB_RUN_IN_DEVICE_MODE	*/

#ifdef ucPORT_USB_RUN_IN_HOST_MODE
/*
 * USB host class RTOS task. Copied from stm32cubide example.
 */
static void vUsbTask(void *param)
{
  (void) param;

  while (1)
  {
	  vPortEnterCritical();
	    MX_USB_HOST_Process();
	    vPortExitCritical();
	    if (Appli_state == APPLICATION_READY)
	    {
	    	CDC_HANDLE();
	    }
  }
}
#endif	/*	ucPORT_USB_RUN_IN_HOST_MODE	*/


/*******************************************************************************
 * API functions:
 ******************************************************************************/
/*
 * See header for info.
 */
void vPort_USB_initHardware(void)
{
	vInit();

	/*	Initialize USB tasks	*/
	xUsbTask = xTaskCreateStatic(
			vUsbTask,
			"usb",
			USB_STACK_SIZE,
			NULL,
			configMAX_PRIORITIES - 2,
			pxUsbTaskStack,
			&xUsbTaskStatic	);
}

#ifdef ucPORT_INTERRUPT_IRQ_DEF_USB
#ifdef ucPORT_USB_RUN_IN_DEVICE_MODE
//--------------------------------------------------------------------+
// Forward USB interrupt events to TinyUSB IRQ Handler
//--------------------------------------------------------------------+
void OTG_FS_IRQHandler(void) {
  tud_int_handler(0);
}

void OTG_FS_WKUP_IRQHandler(void) {
  tud_int_handler(0);
}
#endif	/*	ucPORT_USB_RUN_IN_DEVICE_MODE	*/

#ifdef ucPORT_USB_RUN_IN_HOST_MODE
//--------------------------------------------------------------------+
// Forward USB interrupt events to TinyUSB IRQ Handler
//--------------------------------------------------------------------+
void OTG_FS_IRQHandler(void)
{
	extern HCD_HandleTypeDef hhcd_USB_OTG_FS;
	HAL_HCD_IRQHandler(&hhcd_USB_OTG_FS);
}

void OTG_FS_WKUP_IRQHandler(void)
{

}
#endif	/*	ucPORT_USB_RUN_IN_HOST_MODE	*/

#endif	/*	ucPORT_INTERRUPT_IRQ_DEF_USB	*/

#endif	/*	ucPORT_USB_ENABLE	*/

#endif /* Target checking */
