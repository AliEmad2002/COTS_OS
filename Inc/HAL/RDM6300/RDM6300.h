/*
 * RDM6300.h
 *
 *  Created on: Sep 14, 2023
 *      Author: Ali Emad
 *
 * RFID reader (RDM6300) driver.
 */

#ifndef COTS_OS_INC_HAL_RDM6300_RDM6300_H_
#define COTS_OS_INC_HAL_RDM6300_RDM6300_H_

#include "HAL/RDM6300/RDM6300_Config.h"

typedef struct{
	/*	PUBLIC	*/
	uint8_t pucData[5];
}xHOS_RDM6300_ID_t;

typedef struct{
	/*	PRIVATE	*/
	uint8_t ucSOF;
	char pcIDStr[10];
	char pcCheckSumStr[2];
	uint8_t ucEOF;
}xHOS_RDM6300_Frame_t;

typedef struct{
	/*	PUBLIC	*/
	uint8_t ucUartUnitNumber;

	uint8_t ucIntPort;
	uint8_t ucIntPin;

	/*	PRIVATE	*/
	StackType_t pxTaskStack[configMINIMAL_STACK_SIZE];
	StaticTask_t xTaskStatic;
	TaskHandle_t xTask;

	StaticSemaphore_t xNewReadSemaphoreStatic;
	SemaphoreHandle_t xNewReadSemaphore;

	uint8_t pucReadQueueMemory[uiCONF_RDM6300_READ_QUEUE_LEN * sizeof(xHOS_RDM6300_ID_t)];
	StaticQueue_t xReadQueueStatic;
	QueueHandle_t xReadQueue;

	xHOS_RDM6300_Frame_t xFrame;

	xHOS_RDM6300_ID_t xTempID;
}xHOS_RDM6300_t;


/*
 * Initializes handle.
 *
 * Notes:
 * 		-	UART driver must be previously initialized.
 */
void vHOS_RDM6300_init(xHOS_RDM6300_t* pxHandle);

/*
 * Gets new reading (ID of newly scanned tag).
 *
 * Notes:
 * 		-	If a new reading was previously received, or was received within the
 * 			given timeout, the ID is copied to "pxID" and function returns 1.
 * 			Otherwise, it returns 0.
 */
uint8_t ucHOS_RDM6300_getNewReading(	xHOS_RDM6300_t* pxHandle,
										xHOS_RDM6300_ID_t* pxID,
										TickType_t xTimeout);


#endif /* COTS_OS_INC_HAL_RDM6300_RDM6300_H_ */

