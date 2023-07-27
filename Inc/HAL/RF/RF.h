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

typedef struct{
	/*	PUBLIC	*/
	uint32_t uiMsTimePerBit;

	uint8_t ucTxPort;
	uint8_t ucTxPin;

	uint8_t ucRxPort;
	uint8_t ucRxPin;

	uint8_t ucSelfAddress;

	uint8_t ucRxCompleteFalg : 1;	// Set by driver after receiving a complete frame,
									// must be cleared by user after reading the
									// RxBuffer. If not, overrun flag will be raised.

	uint8_t ucTxEmptyFalg : 1;		// (Read only) Set by driver after transmitting
									// a complete frame. And cleared by driver on
									// the begging of transmitting a new frame.
									// If driver was ordered to transmit while this
									// flag is not set, overrun flag will be raised,
									// current transmission will be canceled and new
									// frame transmission will start.

	uint8_t ucOverrunFlag : 1;		// Set by driver, cleared by user.

	uint8_t ucRxCompleteCallbackEnable : 1;
	uint8_t ucTxEmptyCallbackEnable : 1;
	uint8_t ucOverrunCallbackEnable : 1;

	void (*pfRxCompleteCallback)(void*);
	void* pvRxCompleteCallbackParams;

	void (*pfTxEmptyCallback)(void*);
	void* pvTxEmptyCallbackParams;

	void (*pfOverrunCallback)(void*);
	void* pvOverrunCallbackParams;

	uint8_t pucRxBuffer[15];	// (Read only) Rx buffer. Gets updated
								// at the end of every frame reception.

	/*	PRIVATE	*/
//	uint64_t pulRxShiftRegister[3];
//	uint64_t pulTxShiftRegister[3];
	uint8_t ucRxShiftRegister;
	uint8_t ucTxShiftRegister;

	uint8_t ucTxNRemaining;

	uint8_t ucEdgeDetectionFlag;

	StaticSemaphore_t xDummySemaphoreStatic;
	SemaphoreHandle_t xDummySemaphore;

	StackType_t puxRxPhyTaskStack[configMINIMAL_STACK_SIZE];
	StaticTask_t xRxPhyTaskStatic;
	TaskHandle_t xRxPhyTask;

	StackType_t puxTxPhyTaskStack[configMINIMAL_STACK_SIZE];
	StaticTask_t xTxPhyTaskStatic;
	TaskHandle_t xTxPhyTask;
}xHOS_RF_t;

typedef struct{
	uint8_t ucDestAddress;
	uint8_t ucDataLen;
	uint8_t* pucData;
}xHOS_RF_TransmissionInfo_t;

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
void vHOS_RF_send(xHOS_RF_t* pxHandle, xHOS_RF_TransmissionInfo_t* pxInfo);





















#endif /* COTS_OS_INC_HAL_RF_RF_H_ */
