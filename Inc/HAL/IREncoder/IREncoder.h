/*
 * IREncoder.h
 *
 *  Created on: Sep 5, 2023
 *      Author: ali20
 */

#ifndef INC_HAL_IRENCODER_IRENCODER_H_
#define INC_HAL_IRENCODER_IRENCODER_H_

typedef struct{
	/*		PRIVATE		*/
	StackType_t puxTaskStack[configMINIMAL_STACK_SIZE];
	StaticTask_t xTaskStatic;
	TaskHandle_t xTask;

	SemaphoreHandle_t xIncSemaphore;
	StaticSemaphore_t xIncSemaphoreStatic;

	SemaphoreHandle_t xCntSemaphore;
	StaticSemaphore_t xCntSemaphoreStatic;

	int32_t iCount;
	int32_t iIncrementer;

	/*		PUBLIC		*/
	uint8_t ucAPort;
	uint8_t ucAPin;
	uint8_t ucBPort;
	uint8_t ucBPin;

	uint8_t ucAdcUnitNumber;

	int32_t iVoltageBottomThreshold;	//	in mV
	int32_t iVoltageTopThreshold;		//	in mV

	uint32_t uiSampleTimeMs;
}xHOS_IREncoder_t;


void vHOS_IREncoder_init(xHOS_IREncoder_t* pxHandle);

void vHOS_IREncoder_setIncrementer(xHOS_IREncoder_t* pxHandle, int32_t iIncrementer);

int32_t iHOS_IREncoder_getCounter(xHOS_IREncoder_t* pxHandle);




















#endif /* INC_HAL_IRENCODER_IRENCODER_H_ */

