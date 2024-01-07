/*
 * CalibratableAmplifier.c
 *
 *  Created on: Dec 30, 2023
 *      Author: Ali Emad
 */

/*	LIB	*/
#include "stdint.h"

/*	RTOS	*/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "RTOS_PRI_Config.h"

/*	MCAL	*/
#include "MCAL_Port/Port_DIO.h"
#include "MCAL_Port/Port_ADC.h"

/*	HAL	*/
#include "HAL/ADC/ADC.h"

/*	SELF	*/
#include "HAL/CalibratableAmplifier/CalibratableAmplifier.h"


/*******************************************************************************
 * Helping functions / macros:
 ******************************************************************************/
/*	Output high state on a given pin	*/
static void vOutputHigh(uint8_t ucPort, uint8_t ucPin)
{
	/*	Should be critical section to avoid shorting connections in the circuit	*/
	taskENTER_CRITICAL();
	{
		vPort_DIO_initPinOutput(ucPort, ucPin);
		vPORT_DIO_WRITE_PIN(ucPort, ucPin, 1);
	}
	taskEXIT_CRITICAL();
}

/*	Output low state on a given pin	*/
static void vOutputLow(uint8_t ucPort, uint8_t ucPin)
{
	/*	Should be critical section to avoid shorting connections in the circuit	*/
	taskENTER_CRITICAL();
	{
		vPort_DIO_initPinOutput(ucPort, ucPin);
		vPORT_DIO_WRITE_PIN(ucPort, ucPin, 0);
	}
	taskEXIT_CRITICAL();
}

/*	Disconnects output on a given pin	*/
static inline void vOutputDisconnect(uint8_t ucPort, uint8_t ucPin)
{
	vPort_DIO_initPinInput(ucPort, ucPin, 0);
}

/*
 * Switches control signals between normal and calibration modes.
 * 0==>normal, 1==>calibration.
 */
static void vSwitchCtrl(xHOS_CalibratableAmplifier_t* pxHandle, uint8_t ucMode)
{
	uint8_t ucHighMask;
	uint8_t ucLowMask;

	if (ucMode == 0)
	{
		ucHighMask = pxHandle->ucAmpOnCtrlHighStateMask;
		ucLowMask = pxHandle->ucAmpOnCtrlLowStateMask;
	}
	else
	{
		ucHighMask = pxHandle->ucAmpCalibCtrlHighStateMask;
		ucLowMask = pxHandle->ucAmpCalibCtrlLowStateMask;
	}

	/*	Should be critical section to avoid shorting connections in the circuit	*/
	taskENTER_CRITICAL();
	{
		for(uint8_t i = 0; i < pxHandle->ucNumberOfCtrlPins; i++)
		{
			if (ucHighMask & (1 << i))
				vOutputHigh(pxHandle->pucCtrlPortArr[i], pxHandle->pucCtrlPinArr[i]);

			else if (ucLowMask & (1 << i))
				vOutputLow(pxHandle->pucCtrlPortArr[i], pxHandle->pucCtrlPinArr[i]);

			else
				vOutputDisconnect(pxHandle->pucCtrlPortArr[i], pxHandle->pucCtrlPinArr[i]);
		}
	}
	taskEXIT_CRITICAL();
}

/*******************************************************************************
 * RTOS task:
 ******************************************************************************/
static void vTask(void* pvParams)
{
	xHOS_CalibratableAmplifier_t* pxHandle = (xHOS_CalibratableAmplifier_t*)pvParams;

	TickType_t xLastWakeTime = xTaskGetTickCount();

	while(1)
	{
		/*	Lock HW	*/
		xSemaphoreTake(pxHandle->xMutex, portMAX_DELAY);

		/*	Switch control pins to the calibration state	*/
		vSwitchCtrl(pxHandle, 1);

		/*	Wait for HW to be stable after switching	*/
		vTaskDelay(pdMS_TO_TICKS(pxHandle->uiSwitchTimeMs));

		/*	Lock ADC unit	*/
		ucHOS_ADC_lockUnit(pxHandle->ucAdcUnitNumber, portMAX_DELAY);

		/*	Read amplifier's output	*/
		pxHandle->uiOffsetRaw = uiHOS_ADC_readChannelBlocking(
				pxHandle->ucAdcUnitNumber, pxHandle->ucAdcChannelNumber	);

		/*	Unlock ADC unit	*/
		vHOS_ADC_unlockUnit(pxHandle->ucAdcUnitNumber);

		/*	Switch control pins to the normal state	*/
		vSwitchCtrl(pxHandle, 0);

		/*	Wait for HW to be stable after switching	*/
		vTaskDelay(pdMS_TO_TICKS(pxHandle->uiSwitchTimeMs));

		/*	Unlock HW	*/
		xSemaphoreGive(pxHandle->xMutex);

		/*	Initial calibration is done	*/
		pxHandle->ucIsInitialCalibDone = 1;

		/*	Delay until next period	*/
		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(pxHandle->uiCalibTimePeriodMs));
	}
}

/*******************************************************************************
 * API functions:
 ******************************************************************************/
void vHOS_CalibratableAmplifier_init(	xHOS_CalibratableAmplifier_t* pxHandle,
										uint32_t uiSampleTime	)
{
	/*	Offset is initially zero	*/
	pxHandle->uiOffsetRaw = 0;

	/*	Initial calibration not done yet	*/
	pxHandle->ucIsInitialCalibDone = 0;

	/*	Set sample time	*/
	vHOS_ADC_setSampleTime(
			pxHandle->ucAdcUnitNumber,
			pxHandle->ucAdcChannelNumber,
			uiSampleTime	);

	/*	Create circuit mutex	*/
	pxHandle->xMutex = xSemaphoreCreateMutexStatic(&pxHandle->xMutexStatic);
	xSemaphoreGive(pxHandle->xMutex);

	/*	Create calibration task	*/
	pxHandle->xTask = xTaskCreateStatic(
			vTask,
			"cAmp",
			uiHOS_CALIBRATABLEAMPLIFIER_HANDLE_TANSK_STACK_SIZE,
			(void*)pxHandle,
			configHOS_SOFT_REAL_TIME_TASK_PRI,
			pxHandle->pxStack,
			&pxHandle->xTaskStatic	);
}

/*
 * See header for info.
 */
int32_t iHOS_CalibratableAmplifier_read(
		xHOS_CalibratableAmplifier_t* pxHandle,
		TickType_t xTimeout	)
{
	/*	Lock HW	*/
	xSemaphoreTake(pxHandle->xMutex, portMAX_DELAY);

	/*	Lock ADC unit	*/
	ucHOS_ADC_lockUnit(pxHandle->ucAdcUnitNumber, portMAX_DELAY);

	/*	Read amplifier's output	*/
	int32_t iAmpOutRaw = uiHOS_ADC_readChannelBlocking(
			pxHandle->ucAdcUnitNumber, pxHandle->ucAdcChannelNumber	);

	/*	Unlock ADC unit	*/
	vHOS_ADC_unlockUnit(pxHandle->ucAdcUnitNumber);

	/*	Unlock HW	*/
	xSemaphoreGive(pxHandle->xMutex);

	/*	Subtract offset	*/
	iAmpOutRaw -= (int32_t)pxHandle->uiOffsetRaw;

	ucHOS_ADC_lockUnit(ucPORT_ADC_VREFINT_UNIT_NUMBER, portMAX_DELAY);

	uint32_t uiVrefIntRaw = uiHOS_ADC_readChannelBlocking(
			ucPORT_ADC_VREFINT_UNIT_NUMBER, ucPORT_ADC_VREFINT_CH_NUMBER	);

	vHOS_ADC_unlockUnit(ucPORT_ADC_VREFINT_UNIT_NUMBER);

	/*	Convert reading to voltage	*/
	int32_t iAmpOutUv = iHOS_ADC_getVoltageCalib(iAmpOutRaw, uiVrefIntRaw);

	/*	Divide by gain	*/
	int32_t iAmpInUv = (1000000 * (int64_t)iAmpOutUv) / pxHandle->iGainUVPV;

	return iAmpInUv;
}










