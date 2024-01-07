/*
 * Relay.h
 *
 *  Created on: Jan 6, 2024
 *      Author: Ali Emad
 *
 * This driver controls relay control pin as requested, with projecting relay coil
 * from generating high current by inserting a minimum delay between switches.
 */

#ifndef COTS_OS_INC_HAL_RELAY_RELAY_H_
#define COTS_OS_INC_HAL_RELAY_RELAY_H_

typedef struct{
	/*		PUBLIC		*/
	/*	Minimum protection delay time between two consecutive switches (in ms)	*/
	uint32_t uiProtectionDelayMs;

	/*	Pin on which relay control transistor is connected	*/
	uint8_t ucPort;
	uint8_t ucPin;

	/*	Active control pin state (Pin state which closes relay circuit)	*/
	uint8_t ucActiveState;

	/*		PRIVATE		*/
	/*	Shadow register of the requested state	*/
	uint8_t ucRequestedState;

	/*	Current state	*/
	uint8_t ucCurrentState;

	/*	Mutex	*/
	SemaphoreHandle_t xMutex;
	StaticSemaphore_t xMutexStatic;

	/*	USE QUEUE INSTEAD!!!!!!	*/
	/*	Control semaphore	*/
	SemaphoreHandle_t xCtrlSemphr;
	StaticSemaphore_t xCtrlSemphrStatic;

	/*	Task	*/
	TaskHandle_t xTask;
	StaticTask_t xTaskStatic;
	StackType_t pxTaskStack[configMINIMAL_STACK_SIZE];
}xHOS_Relay_t;

/*
 * Initializes handle.
 *
 * Notes:
 * 		-	All public parameters must be initialized first.
 */
void vHOS_Relay_init(xHOS_Relay_t* pxHandle);

/*
 * Locks handle for the caller task.
 */
uint8_t ucHOS_Relay_lock(xHOS_Relay_t* pxHandle, TickType_t xTimeout);

/*
 * Unlocks handle.
 *
 * Notes:
 * 		-	Must be called by the task which have already locked the same handle.
 * 			(Mutex based)
 */
void vHOS_Relay_unlock(xHOS_Relay_t* pxHandle);

/*
 * Send switch request.
 *
 * Notes:
 * 		-	"ucRelayNewState":	0==> Opens relay circuit.
 * 								1==> Closes relay circuit.
 *
 * 		-	Caller task should have previously locked the same handle. Resource
 * 			management is user's responsibility.
 */
void vHOS_Relay_switch(xHOS_Relay_t* pxHandle, uint8_t ucRelayNewState);







#endif /* COTS_OS_INC_HAL_RELAY_RELAY_H_ */
