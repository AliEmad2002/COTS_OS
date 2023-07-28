/*
 * RF.h
 *
 *  Created on: Jul 26, 2023
 *      Author: Ali Emad
 *
 * Notes:
 * 		-	This driver implement ASK-RF transreceiver communication.
 *
 *		-	Transmitter is expected to be normally not outputting carrier waves,
 *			that's when it get an input of low level voltage. And is expected to
 *			keep outputting carrier waves when it get an input of high level voltage.
 *
 * 		-	Receiver's circuit is expected to be normally outputting low level
 * 			voltage, with a rising edge upon detecting a carrier wave.
 */

#ifndef COTS_OS_INC_HAL_RF_RF_H_
#define COTS_OS_INC_HAL_RF_RF_H_

#include "semphr.h"
#include "HAL/RF/RF_config.h"

/*	Size of the additional bytes (SOF, DestAddress, SrcAddress, CRC and EOF)	*/
#define uiRF_NUMBER_OF_ADDITIONAL_BYTES			(7)

/*	Full frame size	*/
#define uiRF_FRAME_SIZE_IN_BYTES	\
	(uiRF_NUMBER_OF_ADDITIONAL_BYTES + uiRF_DATA_BYTES_PER_FRAME)


typedef struct{
	/*	PUBLIC	*/
	uint8_t ucTxPort;
	uint8_t ucTxPin;

	uint8_t ucRxPort;
	uint8_t ucRxPin;

	uint8_t ucSelfAddress;

	uint8_t ucRxCompleteFalg : 1;	// (Read only) Set by driver after receiving
									// a complete frame, gets cleared when user
									// calls "clearRxCompleteFlag()"after reading the
									// RxBuffer. If not called, and new frame was
									// received, overrun flag will be raised.
									// And receiving tasks are suspended until user
			 	 	 	 	 	 	 // re-enables them using "vHOS_RF_enable()"

	uint8_t ucTxEmptyFalg : 1;		// (Read only) Set by driver after transmitting
									// a complete frame. And cleared by driver on
									// the begging of transmitting a new frame.
									// If driver was ordered to transmit while this
									// flag is not set, overrun flag will be raised,
									// and new transmission will be canceled.

	uint8_t ucOverrunFlag : 1;		// Set by driver, cleared by user.

	uint8_t ucAckFlag : 1;			// (Read only) Set by driver on reception of
									// an ACK frame, cleared when user calls:
									// "clearAckFlag()" function.

	uint8_t pucRxBuffer[uiRF_DATA_BYTES_PER_FRAME];	// (Read only) Rx buffer. Gets updated
													// at the end of every frame reception.

	uint8_t ucSrcAddress;							// (Read only) source address of the
													// just received frame. Gets updated
													// at the end of every frame reception.

	uint16_t usRxCRC;								// (Read only) CRC of the
													// just received frame. Gets updated
													// at the end of every frame reception.

	/*	PRIVATE	*/
	uint8_t pucRxShiftRegister[uiRF_FRAME_SIZE_IN_BYTES];
	uint8_t pucTxShiftRegister[uiRF_FRAME_SIZE_IN_BYTES];

	uint32_t uiTxNRemaining;

	uint8_t ucEdgeDetectionFlag;

	StaticSemaphore_t xTxEmptySemaphoreStatic;
	SemaphoreHandle_t xTxEmptySemaphore;

	StaticSemaphore_t xRxCompleteSemaphoreStatic;
	SemaphoreHandle_t xRxCompleteSemaphore;

	StaticSemaphore_t xAckSemaphoreStatic;
	SemaphoreHandle_t xAckSemaphore;

	StaticSemaphore_t xDummySemaphoreStatic;
	SemaphoreHandle_t xDummySemaphore;

	StaticSemaphore_t xPhySemaphoreStatic;
	SemaphoreHandle_t xPhySemaphore;

	StackType_t puxRxPhyTaskStack[configMINIMAL_STACK_SIZE];
	StaticTask_t xRxPhyTaskStatic;
	TaskHandle_t xRxPhyTask;

	StackType_t puxTxPhyTaskStack[configMINIMAL_STACK_SIZE];
	StaticTask_t xTxPhyTaskStatic;
	TaskHandle_t xTxPhyTask;

	StackType_t puxRxTaskStack[configMINIMAL_STACK_SIZE];
	StaticTask_t xRxTaskStatic;
	TaskHandle_t xRxTask;
}xHOS_RF_t;


/*
 * Initializes an RF transreceiver.
 *
 * Notes:
 * 		-	All public parameters of the handle must be initialized previously.
 * 		-	This function must be called before scheduler start.
 */
void vHOS_RF_init(xHOS_RF_t* pxHandle);

/*
 * Enables an RF transreceiver.
 *
 * Notes:
 * 		-	Handles are initially disabled.
 */
void vHOS_RF_enable(xHOS_RF_t* pxHandle);

/*
 * Disables an RF transreceiver.
 *
 * Notes:
 * 		-	Handles are initially disabled.
 */
void vHOS_RF_disable(xHOS_RF_t* pxHandle);

/*
 * Sends new data.
 */
void vHOS_RF_send(	xHOS_RF_t* pxHandle,
					uint8_t ucDestAddress,
					uint8_t* pucData,
					uint32_t uiDataSizeInBytes,
					uint16_t usCRC	);

/*
 * Sends an ACK frame.
 */
void vHOS_RF_sendAck(xHOS_RF_t* pxHandle, uint8_t ucDestAddress);

/*
 * Blocks the calling task until TxEmpty flag is raised.
 * (Current transmission is done).
 */
void vHOS_RF_blockUntilTxEmpty(xHOS_RF_t* pxHandle);

/*
 * Blocks the calling task until RxComplete flag is raised (Current reception is
 * done), with a given timeout.
 *
 * Returns pdTRUE if flag was raised during the timeout, and pdFALSE otherwise.
 */
BaseType_t xHOS_RF_blockUntilRxComplete(xHOS_RF_t* pxHandle, TickType_t xTimeoutTicks);

/*
 * Blocks the calling task until ACK flag is raised (An ACK frame was received),
 * with a given timeout.
 *
 * Returns pdTRUE if flag was raised during the timeout, and pdFALSE otherwise.
 */
BaseType_t xHOS_RF_blockUntilAckReceived(xHOS_RF_t* pxHandle, TickType_t xTimeoutTicks);

/*	Clears the RxComplete flag	*/
void vHOS_RF_clearRxComplete(xHOS_RF_t* pxHandle);

/*	Clears the ACK flag	*/
void vHOS_RF_clearAck(xHOS_RF_t* pxHandle);

















#endif /* COTS_OS_INC_HAL_RF_RF_H_ */
