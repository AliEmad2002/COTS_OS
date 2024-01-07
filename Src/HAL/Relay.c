/*
 * Relay.c
 *
 *  Created on: Jan 6, 2024
 *      Author: Ali Emad
 */

/*	RTOS	*/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "RTOS_PRI_Config.h"

/*	LIB	*/
#include <stdint.h>

/*	MCAL	*/
#include "MCAL_Port/Port_DIO.h"

/*	SELF	*/
#include "HAL/Relay/Relay.h"

/*******************************************************************************
 * Helping macros:
 ******************************************************************************/
#define vCLOSE_CIRCUIT(pxHandle)	\
	(	vPORT_DIO_WRITE_PIN((pxHandle)->ucPort, (pxHandle)->ucPin, (pxHandle)->ucActiveState)	)

#define vOPEN_CIRCUIT(pxHandle)	\
	(	vPORT_DIO_WRITE_PIN((pxHandle)->ucPort, (pxHandle)->ucPin, !(pxHandle)->ucActiveState)	)

/*******************************************************************************
 * RTOS task:
 ******************************************************************************/
static void vTask(void* pvParams)
{
	xHOS_Relay_t* pxHandle = (xHOS_Relay_t*)pvParams;

	while(1)
	{
		/*	Block until control semaphore is released 	*/
		xSemaphoreTake(pxHandle->xCtrlSemphr, portMAX_DELAY);

		/*	Switch the relay	*/
		pxHandle->ucCurrentState = pxHandle->ucRequestedState;
		if (pxHandle->ucRequestedState == 0)	vOPEN_CIRCUIT(pxHandle);
		else									vCLOSE_CIRCUIT(pxHandle);

		/*	Protection delay	*/
		vTaskDelay(pdMS_TO_TICKS(pxHandle->uiProtectionDelayMs));
	}
}


/*******************************************************************************
 * API functions:
 ******************************************************************************/
/*
 * See header for info.
 */
void vHOS_Relay_init(xHOS_Relay_t* pxHandle)
{
	/*	Initialize control pin	*/
	vPort_DIO_initPinOutput(pxHandle->ucPort, pxHandle->ucPin);
	vOPEN_CIRCUIT(pxHandle);

	/*	Requested state is initially open circuit	*/
	pxHandle->ucRequestedState = 0;
	pxHandle->ucCurrentState = 0;

	/*	Create control semaphore	*/
	pxHandle->xCtrlSemphr = xSemaphoreCreateBinaryStatic(&pxHandle->xCtrlSemphrStatic);
	xSemaphoreTake(pxHandle->xCtrlSemphr, 0);

	/*	Create mutex	*/
	pxHandle->xMutex = xSemaphoreCreateMutexStatic(&pxHandle->xMutexStatic);
	xSemaphoreGive(pxHandle->xMutex);

	/*	Create control task	*/
	pxHandle->xTask = xTaskCreateStatic(	vTask,
											"Relay",
											configMINIMAL_STACK_SIZE,
											(void*)pxHandle,
											configHOS_SOFT_REAL_TIME_TASK_PRI,
											pxHandle->pxTaskStack,
											&pxHandle->xTaskStatic	);
}

/*
 * See header for info.
 */
uint8_t ucHOS_Relay_lock(xHOS_Relay_t* pxHandle, TickType_t xTimeout)
{
	return xSemaphoreTake(pxHandle->xMutex, xTimeout);
}

/*
 * See header for info.
 */
void vHOS_Relay_unlock(xHOS_Relay_t* pxHandle)
{
	xSemaphoreGive(pxHandle->xMutex);
}

/*
 * See header for info.
 */
void vHOS_Relay_switch(xHOS_Relay_t* pxHandle, uint8_t ucRelayNewState)
{
	/*	If requested state is same as current state, return	*/
	if (pxHandle->ucCurrentState == ucRelayNewState)
		return;

	/*	Write state to the shadow register	*/
	pxHandle->ucRequestedState = ucRelayNewState;

	/*	Give control semaphore	*/
	xSemaphoreGive(pxHandle->xCtrlSemphr);
}







