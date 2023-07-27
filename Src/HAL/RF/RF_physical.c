/*
 * RF_physical.c
 *
 *  Created on: Jul 26, 2023
 *      Author: Ali Emad
 */

/*	LIB	*/
#include "stdint.h"
#include "stdio.h"

/*	MCAL (ported)	*/
#include "MCAL_Port/Port_Breakpoint.h"
#include "MCAL_Port/Port_DIO.h"
#include "MCAL_Port/Port_EXTI.h"
#include "MCAL_Port/Port_Interrupt.h"

/*	OS	*/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "RTOS_PRI_Config.h"

/*	HAL	*/

/*	SELF	*/
#include "HAL/RF/RF_private.h"
#include "HAL/RF/RF.h"

/*******************************************************************************
 * Helping functions / macros:
 ******************************************************************************/
#define vSHIFT_RIGHT(pulShiftReg)											     \
{                                                                                \
	(pulShiftReg)[0] = ((pulShiftReg)[0] >> 1) | (((pulShiftReg)[1] & 1) << 63); \
	(pulShiftReg)[1] = ((pulShiftReg)[1] >> 1) | (((pulShiftReg)[2] & 1) << 63); \
	(pulShiftReg)[2] = ((pulShiftReg)[2] >> 1);                                  \
}

#define vSHIFT_LEFT(pulShiftReg)											     \
{                                                                                \
	(pulShiftReg)[2] = ((pulShiftReg)[2] << 1) | (((pulShiftReg)[1] >> 63) & 1); \
	(pulShiftReg)[1] = ((pulShiftReg)[1] << 1) | (((pulShiftReg)[0] >> 63) & 1); \
	(pulShiftReg)[0] = ((pulShiftReg)[0] << 1);									 \
}

/*******************************************************************************
 * Task functions:
 ******************************************************************************/
static void vTxTask(void* pvParams)
{
	xHOS_RF_t* pxHandle = (xHOS_RF_t*)pvParams;

	uint8_t ucNewBit;

	/*	Task is initially suspended	*/
	vTaskSuspend(pxHandle->xTxPhyTask);

	TickType_t xLastWakeTime = xTaskGetTickCount();
	while(1)
	{
		/*	If done transmitting all bits, task is suspend	*/
		if (pxHandle->ucTxNRemaining == 0)
		{
			vTaskSuspend(pxHandle->xTxPhyTask);
			continue;
		}

		/*	If there's still un-transmitted bits, get value of the MSB	*/
		ucNewBit = (pxHandle->ucTxShiftRegister >> 7) & 1;

		/*
		 * If MSB is 1, generate pulse on the transmitter, shift left the register
		 * so that the following bit is now the MSB, and decrement number of remaining
		 * bits.
		 */
		if (ucNewBit == 1)
		{
			vPort_DIO_writePin(pxHandle->ucTxPort, pxHandle->ucTxPin, 1);
			vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1));
			vPort_DIO_writePin(pxHandle->ucTxPort, pxHandle->ucTxPin, 0);
			vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1));

			pxHandle->ucTxShiftRegister = pxHandle->ucTxShiftRegister << 1;
			pxHandle->ucTxNRemaining--;
		}

		/*
		 * Otherwise, if MSB is 0, don't pulse the transmitter, and write MSB
		 * to 1, so that the following MSB is 1 (stuffing bit).
		 */
		else
		{
			vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(2));

			pxHandle->ucTxShiftRegister = pxHandle->ucTxShiftRegister | (1 << 7);
		}
	}
}

static void vRxTask(void* pvParams)
{
	xHOS_RF_t* pxHandle = (xHOS_RF_t*)pvParams;

	uint8_t ucNewBit;

	/*	Task is initially suspended	*/
	vTaskSuspend(pxHandle->xRxPhyTask);

	while(1)
	{
		/*	If an edge was detected, the new received bit is 1	*/
		if (pxHandle->ucEdgeDetectionFlag == 1)
		{
			pxHandle->ucEdgeDetectionFlag = 0;
			ucNewBit = 1;
		}

		/*
		 * Otherwise, if there was no edges detected, the new received bit is 0.
		 * And task would be block blocked until a new edge is received from the
		 * RF-receiver. This is done to synchronize both Rx-task and Rx-signal.
		 */
		else
		{
			ucNewBit = 0;
			xSemaphoreTake(pxHandle->xDummySemaphore, 0);
			xSemaphoreTake(pxHandle->xDummySemaphore, portMAX_DELAY);

			pxHandle->ucEdgeDetectionFlag = 0;

			/*
			 * wait for a maximum of 3ms, so that a false zero does not get written
			 * (in case start of the loop was executed directly without this delay).
			 */
			xSemaphoreTake(pxHandle->xDummySemaphore, 0);
			xSemaphoreTake(pxHandle->xDummySemaphore, pdMS_TO_TICKS(3));
		}

		/*	Shift left the register, and write the new received bit to register's LSB.	*/
		pxHandle->ucRxShiftRegister = pxHandle->ucRxShiftRegister << 1;
		pxHandle->ucRxShiftRegister |= ucNewBit;

		/*
		 * If the new received bit is 1, wait for a maximum of 3ms, so that a false
		 * zero does not get written (in case start of the loop was executed directly
		 * without this delay).
		*/
		if (ucNewBit == 1)
		{
			xSemaphoreTake(pxHandle->xDummySemaphore, 0);
			xSemaphoreTake(pxHandle->xDummySemaphore, pdMS_TO_TICKS(3));
		}
	}
}

/*******************************************************************************
 * ISR:
 ******************************************************************************/
void fPORT_EXTI_HANDLER_1(void* pvParams)	/*	TODO: add callback functionality to EXTI driver	*/
{
	extern xHOS_RF_t xRF;
	xHOS_RF_t* pxHandle = &xRF;//(xHOS_RF_t*)pvParams;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	pxHandle->ucEdgeDetectionFlag = 1;
	xSemaphoreGiveFromISR(pxHandle->xDummySemaphore, &xHigherPriorityTaskWoken);

	vPORT_EXTI_CLEAR_PENDING_FLAG(pxHandle->ucRxPort, pxHandle->ucRxPin);

	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/*******************************************************************************
 * Private functions:
 ******************************************************************************/
void xHOS_RFPhysical_init(xHOS_RF_t* pxHandle)
{
	/*	Initialize transmitter's pin as output, initially at low level voltage	*/
	vPort_DIO_initPinOutput(pxHandle->ucTxPort, pxHandle->ucTxPin);
	vPort_DIO_writePin(pxHandle->ucTxPort, pxHandle->ucTxPin, 0);

	/*	Initialize receiver's pin as input, with rising edge interrupt, initially disabled	*/
	vPort_DIO_initPinInput(pxHandle->ucRxPort, pxHandle->ucRxPin, 0);
	vPort_EXTI_initLine(pxHandle->ucRxPort, pxHandle->ucRxPin, 1);
	vPort_EXTI_DisableLine(pxHandle->ucRxPort, pxHandle->ucRxPin);

	/*	Initialize receiver's interrupt in the interrupt controller	*/
	vPort_Interrupt_setPriority(
		pxPortInterruptExtiIrqNumberArr[pxHandle->ucRxPin],
		configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1	);

	vPort_Interrupt_enableIRQ(pxPortInterruptExtiIrqNumberArr[pxHandle->ucRxPin]);

	/*	Initialize counters and flags	*/
	pxHandle->ucTxNRemaining = 0;
	pxHandle->ucEdgeDetectionFlag = 0;

	/*	Create dummy semaphore	*/
	pxHandle->xDummySemaphore = xSemaphoreCreateBinaryStatic(&pxHandle->xDummySemaphoreStatic);
	xSemaphoreTake(pxHandle->xDummySemaphore, 0);

	/*	Create RxPhysical task	*/
	static uint8_t ucCreatedObjectsCount = 0;
	char pcRxTaskName[configMAX_TASK_NAME_LEN];
	sprintf(pcRxTaskName, "RF_RxPhy%d", ucCreatedObjectsCount);

	pxHandle->xRxPhyTask = xTaskCreateStatic(	vRxTask,
											pcRxTaskName,
											configMINIMAL_STACK_SIZE,
											(void*)pxHandle,
											configHOS_HARD_REAL_TIME_TASK_PRI,
											pxHandle->puxRxPhyTaskStack,
											&pxHandle->xRxPhyTaskStatic	);

	/*	Create TxPhysical task	*/
	char pcTxTaskName[configMAX_TASK_NAME_LEN];
	sprintf(pcTxTaskName, "RF_TxPhy%d", ucCreatedObjectsCount++);

	pxHandle->xTxPhyTask = xTaskCreateStatic(	vTxTask,
											pcTxTaskName,
											configMINIMAL_STACK_SIZE,
											(void*)pxHandle,
											configHOS_HARD_REAL_TIME_TASK_PRI,
											pxHandle->puxTxPhyTaskStack,
											&pxHandle->xTxPhyTaskStatic	);
}

void xHOS_RFPhysical_enable(xHOS_RF_t* pxHandle)
{
	/*	Enable EXTI line */
	vPort_EXTI_EnableLine(pxHandle->ucRxPort, pxHandle->ucRxPin);

	/*	Resume Rx, Tx tasks	*/
	vTaskResume(pxHandle->xRxPhyTask);
	vTaskResume(pxHandle->xTxPhyTask);
}

void xHOS_RFPhysical_disable(xHOS_RF_t* pxHandle)
{
	/*	Disable EXTI line */
	vPort_EXTI_DisableLine(pxHandle->ucRxPort, pxHandle->ucRxPin);

	/*	Suspend Rx, Tx tasks	*/
	vTaskSuspend(pxHandle->xRxPhyTask);
	vTaskSuspend(pxHandle->xTxPhyTask);
}

















