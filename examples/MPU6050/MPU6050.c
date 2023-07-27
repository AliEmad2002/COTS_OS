/*
 * MPU6050.c
 *
 *  Created on: Jul 26, 2023
 *      Author: Ali Emad
 */
#if 0

/*	LIB	*/
#include <stdint.h>
#include <stdlib.h>

/*	OS	*/
#include "FreeRTOS.h"
#include "task.h"

/*	MCAL	*/
#include "MCAL_Port/Port_HW.h"
#include "MCAL_Port/Port_DIO.h"
#include "MCAL_Port/Port_Breakpoint.h"

/*	HAL_OS	*/
#include "HAL/HAL_OS.h"

/*******************************************************************************
 * Global variables/objects:
 ******************************************************************************/
xHOS_MPU6050_t xMPU;
xHOS_MPU6050_measurement_t xGyroMeasurement;
xHOS_MPU6050_measurement_t xAccelMeasurement;
int32_t iTemp;
float fAlpha = 1.0f / 250.0f;

/*******************************************************************************
 * Tasks stacks and handles:
 ******************************************************************************/
static StackType_t puxTask1Stack[configMINIMAL_STACK_SIZE];
static StaticTask_t xTask1StaticHandle;
static TaskHandle_t xTask1Handle;

/*******************************************************************************
 * Task functions:
 ******************************************************************************/
void vTask1(void* pvParams)
{
	uint8_t ucSuccessful;
	xHOS_MPU6050_testResult_t xTestResults;

	vHOS_I2C_enable(0);

	/*	MPU6050 power stabilization delay	*/
	vTaskDelay(pdMS_TO_TICKS(200));

	ucHOS_MPU6050_enable(&xMPU);

	//vHOS_MPU6050_enterDebuggerMode(&xMPU);

	ucHOS_MPU6050_setGyroFullScaleRange(&xMPU, 3);
	ucHOS_MPU6050_setAccelFullScaleRange(&xMPU, 0);

	ucHOS_MPU6050_setClockSource(&xMPU, 1);

	ucSuccessful = ucHOS_MPU6050_confLpdfAndSampleRate(&xMPU,1, 0);
	if (!ucSuccessful)	{vPort_DIO_writePin(2, 13, 0); while(1);}

//	ucSuccessful = ucHOS_MPU6050_gyroSelfTest(&xMPU, &xTestResults);
//	if (!ucSuccessful)	{vPort_DIO_writePin(2, 13, 0); while(1);}
//
//	ucSuccessful = ucHOS_MPU6050_accelSelfTest(&xMPU, &xTestResults);
//	if (!ucSuccessful)	{vPort_DIO_writePin(2, 13, 0); while(1);}

	ucSuccessful = ucHOS_MPU6050_calibrate(&xMPU);
	if (!ucSuccessful)	{vPort_DIO_writePin(2, 13, 0); while(1);}

	ucSuccessful = ucHOS_MPU6050_enableTiltCalculation(&xMPU);
	if (!ucSuccessful)	{vPort_DIO_writePin(2, 13, 0); while(1);}

	//vTaskSuspend(xTask1Handle);

	while(1)
	{
		ucSuccessful = ucHOS_MPU6050_readAccelMeasurement(&xMPU, &xAccelMeasurement);
		if (!ucSuccessful)	{vPort_DIO_writePin(2, 13, 0); while(1);}

		ucSuccessful = ucHOS_MPU6050_readGyroMeasurement(&xMPU, &xGyroMeasurement);
		if (!ucSuccessful)	{vPort_DIO_writePin(2, 13, 0); while(1);}

		ucSuccessful = ucHOS_MPU6050_readTemperatureMeasurement(&xMPU, &iTemp);
		if (!ucSuccessful)	{vPort_DIO_writePin(2, 13, 0); while(1);}
	}
}

void vApplicationIdleHook( void )
{
   for( ;; )
   {
       //vCoRoutineSchedule();
   }
}

/*******************************************************************************
 * Callbacks:
 ******************************************************************************/


/*******************************************************************************
 * Tasks initialization:
 ******************************************************************************/
void tasks_init(void)
{
	xTask1Handle = xTaskCreateStatic(vTask1, "T1", configMINIMAL_STACK_SIZE, NULL, configHOS_SOFT_REAL_TIME_TASK_PRI, puxTask1Stack, &xTask1StaticHandle);
}

/*******************************************************************************
 * HAL_OS objects initialization:
 ******************************************************************************/
void obj_init(void)
{
	/*	MPU	*/
	xMPU.ucI2CUnitNumber = 0;
	xMPU.ucAdd0State = 0;

	xMPU.ucIsIntConnected = 1;
	xMPU.ucIntPort = 1;
	xMPU.ucIntPin = 12;

	vHOS_MPU6050_init(&xMPU);

	volatile int32_t iPtr0 = (int32_t)&xAccelMeasurement;
	volatile int32_t iPtr1 = (int32_t)&xGyroMeasurement;

	volatile int32_t* piXA = &(xAccelMeasurement.iX);
	volatile int32_t* piYA = &(xAccelMeasurement.iY);
	volatile int32_t* piZA = &(xAccelMeasurement.iZ);

	volatile int32_t* piXG = &(xGyroMeasurement.iX);
	volatile int32_t* piYG = &(xGyroMeasurement.iY);
	volatile int32_t* piZG = &(xGyroMeasurement.iZ);

	volatile int32_t* piXTheta = &(xMPU.xTilt.iX);
	volatile int32_t* piYTheta = &(xMPU.xTilt.iY);
	volatile int32_t* piZTheta = &(xMPU.xTilt.iZ);

	volatile int32_t* piTemp = &iTemp;

	/*	Error LED	*/
	vPort_DIO_initPinOutput(2, 13);
	vPort_DIO_writePin(2, 13, 1);
}

/*******************************************************************************
 * main:
 ******************************************************************************/
int main(void)
{
	/*	init	*/
	vPort_HW_init();
	configASSERT(xHOS_init());
	obj_init();
	tasks_init();

	/*	Run	*/
	vTaskStartScheduler();

	while(1)
	{

	}

	return 0;
}
#endif
