///*
// * IREncoder.c
// *
// *  Created on: Sep 5, 2023
// *      Author: Ali Emad
// */
//
///*	LIB	*/
//#include <stdint.h>
//
///*	FreeRTOS	*/
//#include "FreeRTOS.h"
//#include "task.h"
//#include "semphr.h"
//
///*	MCAL (Ported)	*/
//#include "MCAL_Port/Port_DIO.h"
//#include "MCAL_Port/Port_ADC.h"
//
///*	HAL-OS	*/
//#include "RTOS_PRI_Config.h"
//
///*	SELF	*/
//#include "HAL/IREncoder/IREncoder.h"
//
//
//static inline uint8_t ucGetLevel(xHOS_IREncoder_t* pxHandle, int32_t iSampleVoltage, uint8_t ucPrevLevel)
//{
//	if (iSampleVoltage > pxHandle->iVoltageTopThreshold)
//		return 1;
//
//	if (iSampleVoltage < pxHandle->iVoltageBottomThreshold)
//		return 0;
//
//	return ucPrevLevel;
//}
//
//static inline void vIncrementCounter(xHOS_IREncoder_t* pxHandle)
//{
//	xSemaphoreTake(pxHandle->xCntSemaphore, portMAX_DELAY);
//	xSemaphoreTake(pxHandle->xIncSemaphore, portMAX_DELAY);
//
//	pxHandle->iCount += pxHandle->iIncrementer;
//
//	xSemaphoreGive(pxHandle->xCntSemaphore);
//	xSemaphoreGive(pxHandle->xIncSemaphore);
//}
//
//static void vTask(void* pvParams)
//{
//	xHOS_IREncoder_t* pxHandle = (xHOS_IREncoder_t*)pvParams;
//
//	int32_t iSampleVoltage; // in mV
//	uint8_t ucLevel;
//	uint8_t ucPrevLevel = 1;
//
//	TickType_t xLastWakeTime = xTaskGetTickCount();
//	while(1)
//	{
//		/*	Read ADC sample	*/
//		iSampleVoltage = iPORT_ADC_DIRECT_READ_VOLTAGE(pxHandle->ucAdcUnitNumber);
//
//		/*	Calculate level of the new sample	*/
//		ucLevel = ucGetLevel(pxHandle, iSampleVoltage, ucPrevLevel);
//
//		/*
//		 * If there has been a falling edge, increment the counter.
//		 * (lock CNT & INC first)
//		 */
//		if (ucLevel == 0 && ucPrevLevel == 1)
//			vIncrementCounter(pxHandle);
//
//		/*	Update prevLevel	*/
//		ucPrevLevel = ucLevel;
//
//		/*	Block until next sample	*/
//		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(pxHandle->uiSampleTimeMs));
//	}
//}
//
//void vHOS_IREncoder_init(xHOS_IREncoder_t* pxHandle)
//{
//	/*	Create task	*/
//	pxHandle->xTask = xTaskCreateStatic(
//		vTask,
//		"IRE",
//		configMINIMAL_STACK_SIZE,
//		(void*)pxHandle,
//		configHOS_HARD_REAL_TIME_TASK_PRI,
//		pxHandle->puxTaskStack,
//		&pxHandle->xTaskStatic);
//
//	/*	Create incrementer semaphore	*/
//	pxHandle->xIncSemaphore =
//		xSemaphoreCreateBinaryStatic(&pxHandle->xIncSemaphoreStatic);
//	xSemaphoreGive(pxHandle->xIncSemaphore);
//
//	/*	Create counter semaphore	*/
//	pxHandle->xCntSemaphore =
//		xSemaphoreCreateBinaryStatic(&pxHandle->xCntSemaphoreStatic);
//	xSemaphoreGive(pxHandle->xCntSemaphore);
//
//	/*	Initialize analog input pin	*/
//	vPort_DIO_initPinInput(pxHandle->ucAPort, pxHandle->ucAPin, 0);
//
//	/*	Initialize pull-up input pin	*/
//	vPort_DIO_initPinInput(pxHandle->ucBPort, pxHandle->ucBPin, 1);
//
//	/*	Initialize ADC channel on analog input pin	*/
//	vPort_ADC_InitChannel(pxHandle->ucAdcUnitNumber, pxHandle->ucAPin, ADC_SAMPLETIME_239CYCLES_5);
//
//	/*	Trigger ADC start	*/
//	vPORT_ADC_SW_TRIGGER(pxHandle->ucAdcUnitNumber);
//
//	/*	Initialize counter and its incrementer	*/
//	pxHandle->iCount = 0;
//	pxHandle->iIncrementer = 0;
//}
//
//void vHOS_IREncoder_setIncrementer(xHOS_IREncoder_t* pxHandle, int32_t iIncrementer)
//{
//	/*	Take semaphore (to assure a change is not made in the middle of a sample)	*/
//	xSemaphoreTake(pxHandle->xIncSemaphore, portMAX_DELAY);
//
//	/*	Set to new value	*/
//	pxHandle->iIncrementer = iIncrementer;
//
//	/*	Release semaphore	*/
//	xSemaphoreGive(pxHandle->xIncSemaphore);
//}
//
//int32_t iHOS_IREncoder_getCounter(xHOS_IREncoder_t* pxHandle)
//{
//	/*	Take semaphore (to assure a change is not made in the middle of reading)	*/
//	xSemaphoreTake(pxHandle->xCntSemaphore, portMAX_DELAY);
//
//	/*	Read new value	*/
//	int32_t iRead = pxHandle->iCount;
//
//	/*	Release semaphore	*/
//	xSemaphoreGive(pxHandle->xCntSemaphore);
//
//	/*	Return read value	*/
//	return iRead;
//}
//
//
//
//
//
//
//
//
