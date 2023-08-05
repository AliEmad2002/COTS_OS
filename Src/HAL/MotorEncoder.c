/*
 * MotorEncoder.c
 *
 *  Created on: Aug 2, 2023
 *      Author: Ali Emad
 */

/*	LIB	*/
#include <stdint.h>
#include <stdio.h>

/*	FreeRTOS	*/
#include "FreeRTOS.h"
#include "task.h"

/*	MCAL (Ported)	*/
#include "MCAL_Port/Port_DIO.h"
#include "MCAL_Port/Port_EXTI.h"
#include "MCAL_Port/Port_Interrupt.h"

/*	HAL-OS	*/
#include "RTOS_PRI_Config.h"

/*	SELF	*/
#include "HAL/MotorEncoder/MotorEncoder.h"

/*******************************************************************************
 * ISR callback:
 ******************************************************************************/
static void vCallback(void* pvParams)
{
	xHOS_MotorEncoder_t* pxHandle = (xHOS_MotorEncoder_t*)pvParams;

	/*	if sensor is single sensor, increment pos and return	*/
	if (!pxHandle->ucIsDualSensor)
	{
		pxHandle->iPos += pxHandle->iIncrementer;
		return;
	}

	/*
	 * Otherwise, if it is a dual sensor, read B level and increment / decrement
	 * pos based on the reading.
	 */
	if (ucPort_DIO_readPin(pxHandle->ucBPort, pxHandle->ucBPin))
		pxHandle->iPos += pxHandle->iIncrementer;
	else
		pxHandle->iPos -= pxHandle->iIncrementer;
}

/*******************************************************************************
 * Task function:
 ******************************************************************************/
static void vTask(void* pvParams)
{
	xHOS_MotorEncoder_t* pxHandle = (xHOS_MotorEncoder_t*)pvParams;

	int32_t iPrevPos = 0;

	vTaskSuspend(pxHandle->xTask);

	TickType_t xLastWakeTime = xTaskGetTickCount();
	while(1)
	{
		pxHandle->iSpeed = 	((pxHandle->iPos - iPrevPos) * 1000) /
							pxHandle->uiSpeedUpdatePeriodMs;

		iPrevPos = pxHandle->iPos;

		/*	Task is blocked until next sample time	*/
		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(pxHandle->uiSpeedUpdatePeriodMs));
	}
}

/*******************************************************************************
 * API functions:
 ******************************************************************************/
/*
 * See header file for info.
 */
void vHOS_MotorEncoder_init(	xHOS_MotorEncoder_t* pxHandle,
								uint8_t ucCaptureEdge,
								uint8_t ucSpeedUpdatePri	)
{
	/*	initialize A, B pins	*/
	vPort_DIO_initPinInput(pxHandle->ucAPort, pxHandle->ucAPin, 0);
	if (pxHandle->ucIsDualSensor)
		vPort_DIO_initPinInput(pxHandle->ucBPort, pxHandle->ucBPin, 0);

	/*	initialize EXTI for pin A	*/
	vPort_EXTI_enableLine(pxHandle->ucAPort, pxHandle->ucAPin);
	vPort_EXTI_initLine(pxHandle->ucAPort, pxHandle->ucAPin, ucCaptureEdge);
	vPort_EXTI_setCallback(pxHandle->ucAPort, pxHandle->ucAPin, vCallback, (void*)pxHandle);

	/*	Initialize pin A interrupt from interrupt controller	*/
	vPort_Interrupt_setPriority(
		pxPortInterruptExtiIrqNumberArr[pxHandle->ucAPin],
		configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1	);

	vPort_Interrupt_enableIRQ(pxPortInterruptExtiIrqNumberArr[pxHandle->ucAPin]);

	/*	initialize position and speed	*/
	pxHandle->iPos = 0;
	pxHandle->iSpeed = 0;

	/*	create task	*/
	static uint8_t ucCreatedObjectsCount = 0;
	char pcTaskName[configMAX_TASK_NAME_LEN];
	sprintf(pcTaskName, "Encoder%d", ucCreatedObjectsCount++);

	uint32_t uiPri = ucSpeedUpdatePri ?
			configHOS_HARD_REAL_TIME_TASK_PRI : configHOS_SOFT_REAL_TIME_TASK_PRI;

	pxHandle->xTask = xTaskCreateStatic(	vTask,
											pcTaskName,
											configMINIMAL_STACK_SIZE,
											(void*)pxHandle,
											uiPri,
											pxHandle->puxTaskStack,
											&pxHandle->xTaskStatic	);
}

/*
 * See header file for info.
 */
__attribute__((always_inline)) inline
void vHOS_MotorEncoder_enableSpeedUpdate(xHOS_MotorEncoder_t* pxHandle)
{
	vTaskResume(pxHandle->xTask);
}

/*
 * See header file for info.
 */
__attribute__((always_inline)) inline
void vHOS_MotorEncoder_disableSpeedUpdate(xHOS_MotorEncoder_t* pxHandle)
{
	vTaskSuspend(pxHandle->xTask);
}







