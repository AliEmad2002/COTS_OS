/*
 * RFID.h
 *
 *  Created on: Sep 14, 2023
 *      Author: Ali Emad
 *
 * RFID reader (RFID) driver.
 */

#ifndef COTS_OS_INC_HAL_RFID_RFID_H_
#define COTS_OS_INC_HAL_RFID_RFID_H_

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "HAL/RFID/RFID_Config.h"
#include "HAL/RFID/RFID_Private.h"

#define uiRFID_STACK_SIZE	(300/4)

typedef struct{
	/*	PUBLIC	*/
	uint8_t pucData[5];
}xHOS_RFID_ID_t;

typedef struct{
	/*	PUBLIC	*/
	uint8_t ucUartUnitNumber;

	/*
	 * Type of the used module:
	 * 0: RDM6300.
	 * 1: RF125PS.
	 */
	uint8_t ucType;

	/*	Power enable pin	*/
	uint8_t ucPowerEnPort;
	uint8_t ucPowerEnPin;

	/*	PRIVATE	*/
	StackType_t pxTaskStack[uiRFID_STACK_SIZE];
	StaticTask_t xTaskStatic;
	TaskHandle_t xTask;

	StaticSemaphore_t xNewReadSemaphoreStatic;
	SemaphoreHandle_t xNewReadSemaphore;

	uint8_t pucReadQueueMemory[uiCONF_RFID_READ_QUEUE_LEN * sizeof(xHOS_RFID_ID_t)];
	StaticQueue_t xReadQueueStatic;
	QueueHandle_t xReadQueue;

	uint8_t pucTempFrameQueueMemory[sizeof(xHOS_RFID_LARGER_FRAME_SZ)];
	StaticQueue_t xTempFrameQueueStatic;
	QueueHandle_t xTempFrameQueue;

	xHOS_RFID_ID_t xTempID;

	xHOS_RFID_ID_t xPrevID; // used with RDM6300 only.
}xHOS_RFID_t;


/*
 * Initializes handle.
 *
 * Notes:
 * 		-	UART driver must be previously initialized.
 */
void vHOS_RFID_init(xHOS_RFID_t* pxHandle);

/*	Enables power of given handle	*/
void vHOS_RFID_enablePower(xHOS_RFID_t* pxHandle);

/*	Disables power of given handle	*/
void vHOS_RFID_disablePower(xHOS_RFID_t* pxHandle);

/*
 * Gets new reading (ID of newly scanned tag).
 *
 * Notes:
 * 		-	If a new reading was previously received, or was received within the
 * 			given timeout, the ID is copied to "pxID" and function returns 1.
 * 			Otherwise, it returns 0.
 */
uint8_t ucHOS_RFID_getNewReading(	xHOS_RFID_t* pxHandle,
									xHOS_RFID_ID_t* pxID,
									TickType_t xTimeout);


#endif /* COTS_OS_INC_HAL_RFID_RFID_H_ */

