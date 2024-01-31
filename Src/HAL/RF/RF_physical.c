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
#define uiSHIFT_REG_SIZE_IN_BYTES	uiRF_FRAME_SIZE_IN_BYTES

#define vSHIFT_LEFT(pucSR)											       \
{                                                                          \
	for (uint32_t __i = uiSHIFT_REG_SIZE_IN_BYTES - 1; __i > 0; __i--)     \
	{                                                                      \
		(pucSR)[__i] = ((pucSR)[__i] << 1) | (((pucSR)[__i-1] >> 7) & 1);  \
	}                                                                      \
                                                                           \
	(pucSR)[0] = ((pucSR)[0] << 1);                                        \
}

#define ucGET_MSB(pucSR) 	(((pucSR)[uiSHIFT_REG_SIZE_IN_BYTES - 1] >> 7) & 1)

#define ucSET_MSB(pucSR) 	((pucSR)[uiSHIFT_REG_SIZE_IN_BYTES - 1] |= (1ul << 7))

#define ucIS_SOF_MATCH(pxHandle)		\
	(((xHOS_RF_Frame_t*)((pxHandle)->pucRxShiftRegister))->ucSOF == ucRF_SOF)


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
		/*	If done transmitting all bits, TxEmpty flag is raised and task is suspend	*/
		if (pxHandle->uiTxNRemaining == 0)
		{
			pxHandle->ucTxEmptyFalg = 1;
			xSemaphoreGive(pxHandle->xTxEmptySemaphore);
			vTaskSuspend(pxHandle->xTxPhyTask);
			xLastWakeTime = xTaskGetTickCount();
			continue;
		}

		/*	If just started a new transmission, send the dummy field	*/
		else if (pxHandle->uiTxNRemaining == (uiRF_FRAME_SIZE_IN_BYTES * 8 + uiRF_DUMMY_FIELD_SIZE_IN_BITS))
		{
			for (uint32_t i = 0; i < uiRF_DUMMY_FIELD_SIZE_IN_BITS; i++)
			{
				vPORT_DIO_WRITE_PIN(pxHandle->ucTxPort, pxHandle->ucTxPin, 1);
				vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1));
				vPORT_DIO_WRITE_PIN(pxHandle->ucTxPort, pxHandle->ucTxPin, 0);
				vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1));
			}

			pxHandle->uiTxNRemaining -= uiRF_DUMMY_FIELD_SIZE_IN_BITS;
		}

		/*	If there's still un-transmitted bits, get value of the MSB	*/
		ucNewBit = ucGET_MSB(pxHandle->pucTxShiftRegister);

		/*
		 * If MSB is 1, generate pulse on the transmitter, shift left the register
		 * so that the following bit is now the MSB, and decrement number of remaining
		 * bits.
		 */
		if (ucNewBit == 1)
		{
			vPORT_DIO_WRITE_PIN(pxHandle->ucTxPort, pxHandle->ucTxPin, 1);
			vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1));
			vPORT_DIO_WRITE_PIN(pxHandle->ucTxPort, pxHandle->ucTxPin, 0);
			vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1));

			vSHIFT_LEFT(pxHandle->pucTxShiftRegister);
			pxHandle->uiTxNRemaining--;
		}

		/*
		 * Otherwise, if MSB is 0, don't pulse the transmitter, and write MSB
		 * to 1, so that the following MSB is 1 (stuffing bit).
		 */
		else
		{
			vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(2));

			ucSET_MSB(pxHandle->pucTxShiftRegister);
		}
	}
}

static void vRxTask(void* pvParams)
{
	xHOS_RF_t* pxHandle = (xHOS_RF_t*)pvParams;

	uint8_t ucNewBit;
	uint32_t uiNumberOfBitsReceivedSinceLastSofMatch = 0;

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
		vSHIFT_LEFT(pxHandle->pucRxShiftRegister);
		pxHandle->pucRxShiftRegister[0] |= ucNewBit;

		/*	Increment bit counter	*/
		uiNumberOfBitsReceivedSinceLastSofMatch++;

		/*
		 * Check for SOF (Start Of Frame) field. If it was matching the unique
		 * defined SOF, then a complete frame has been successfully received,
		 * physical layer's receiver task notifies data-link layer's receiver task.
		 * And the first is suspended until the second finishes reading the
		 * frame and resumes it.
		 */
		if (
			ucIS_SOF_MATCH(pxHandle) &&
			uiNumberOfBitsReceivedSinceLastSofMatch >= uiRF_FRAME_SIZE_IN_BYTES * 8)
		{
			uiNumberOfBitsReceivedSinceLastSofMatch = 0;
			xSemaphoreGive(pxHandle->xPhySemaphore);
			vTaskSuspend(pxHandle->xRxPhyTask);
		}

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
static void vISR(void* pvParams)
{
	xHOS_RF_t* pxHandle = (xHOS_RF_t*)pvParams;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	pxHandle->ucEdgeDetectionFlag = 1;
	xSemaphoreGiveFromISR(pxHandle->xDummySemaphore, &xHigherPriorityTaskWoken);

	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/*******************************************************************************
 * Private functions:
 ******************************************************************************/
void xHOS_RFPhysical_init(xHOS_RF_t* pxHandle)
{
	/*	Initialize transmitter's pin as output, initially at low level voltage	*/
	vPort_DIO_initPinOutput(pxHandle->ucTxPort, pxHandle->ucTxPin);
	vPORT_DIO_WRITE_PIN(pxHandle->ucTxPort, pxHandle->ucTxPin, 0);

	/*	Initialize receiver's pin as input, with rising edge interrupt, initially disabled	*/
	vPort_DIO_initPinInput(pxHandle->ucRxPort, pxHandle->ucRxPin, 0);
	vPort_EXTI_setEdge(pxHandle->ucRxPort, pxHandle->ucRxPin, 1);
	vPort_EXTI_setCallback(pxHandle->ucRxPort, pxHandle->ucRxPin, vISR, (void*)pxHandle);
	vPORT_EXTI_DISABLE_LINE(pxHandle->ucRxPort, pxHandle->ucRxPin);

	/*	Initialize receiver's interrupt in the interrupt controller	*/
	uint32_t uiIrqNum = uiPort_EXTI_getIrqNum(0, pxHandle->ucRxPin);
	VPORT_INTERRUPT_SET_PRIORITY(
		uiIrqNum,
		configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1	);

	vPORT_INTERRUPT_ENABLE_IRQ(uiIrqNum);

	/*	Initialize counters and flags	*/
	pxHandle->uiTxNRemaining = 0;
	pxHandle->ucEdgeDetectionFlag = 0;

	/*	Create dummy semaphore	*/
	pxHandle->xDummySemaphore = xSemaphoreCreateBinaryStatic(&pxHandle->xDummySemaphoreStatic);
	xSemaphoreTake(pxHandle->xDummySemaphore, 0);

	/*	Create Phy semaphore	*/
	pxHandle->xPhySemaphore = xSemaphoreCreateBinaryStatic(&pxHandle->xPhySemaphoreStatic);
	xSemaphoreTake(pxHandle->xPhySemaphore, 0);

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
	vPORT_EXTI_ENABLE_LINE(pxHandle->ucRxPort, pxHandle->ucRxPin);

	/*	Resume Rx, Tx tasks	*/
	vTaskResume(pxHandle->xRxPhyTask);
	vTaskResume(pxHandle->xTxPhyTask);
}

void xHOS_RFPhysical_disable(xHOS_RF_t* pxHandle)
{
	/*	Disable EXTI line */
	vPORT_EXTI_DISABLE_LINE(pxHandle->ucRxPort, pxHandle->ucRxPin);

	/*	Suspend Rx, Tx tasks	*/
	vTaskSuspend(pxHandle->xRxPhyTask);
	vTaskSuspend(pxHandle->xTxPhyTask);
}

void xHOS_RFPhysical_startTransmission(xHOS_RF_t* pxHandle)
{
	/*	Reload remaining bits counter	*/
	pxHandle->uiTxNRemaining = uiRF_FRAME_SIZE_IN_BYTES * 8 + uiRF_DUMMY_FIELD_SIZE_IN_BITS;

	/*	Resume Tx task	*/
	vTaskResume(pxHandle->xTxPhyTask);
}















