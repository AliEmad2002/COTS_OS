/*
 * PID_Simulation_Example.c
 *
 *  Created on: Jun 29, 2023
 *      Author: Ali Emad
 *
 * In this example, a first order DC-motor is simulated in the two functions:
 * "fGetSample()" and "vUpdate()". For testing a PID controller object, using
 * stmStudio UI.
 */

//#define PID_SIM_EXAMPLE
#ifdef PID_SIM_EXAMPLE

/*	LIB	*/
#include <stdint.h>
#include <stdlib.h>

/*	OS	*/
#include "FreeRTOS.h"
#include "task.h"

/*	MCAL	*/
#include "stm32f103xb.h"
#include "stm32f1xx_hal.h"
#include "Port/Port_HW.h"
#include "Port/Port_DIO.h"

/*	HAL_OS	*/
#include "Inc/HAL_OS.h"

/*******************************************************************************
 * Global variables/objects:
 ******************************************************************************/
xHOS_PID_t xPID;

static volatile float fYCurrent = 0.0f;
static volatile float fYPrev = 0.0f;
static volatile float fXPrev = 0.0f;

static volatile float* pfYCurrent;
static volatile float* pfSetPoint;
static volatile float* pfKi;
static volatile float* pfKp;
static volatile float* pfKd;
static volatile uint32_t* puiTimeInvervalMs;

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
	vHOS_PID_enable(&xPID);
	vTaskSuspend(xTask1Handle);
	while(1)
	{

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
float fGetSample(void)
{
	return fYCurrent;
}

#define fK			(1.0f)
#define fTau		(0.1f)
#define fT			(1.0f / 1000.0f)

#define fC0			((fK * fT) / (2.0f * fTau + fT))
#define fC1			((-2.0f * fTau + fT) / (2.0f * fTau + fT))

void vUpdate(float fXNew)
{
	float fYNew = fXNew * fC0 + fXPrev * fC0 - fYPrev * fC1;
	fYPrev = fYCurrent;
	fYCurrent = fYNew;
	fXPrev = fXNew;
}

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
	xPID.uiTimeIntervalMs		= 1;
	xPID.fSetPoint				= 10.0f;
	xPID.fKi					= 100.0f;
	xPID.fKp					= 70.0f;
	xPID.fKd					= 0.0f;
	xPID.fIMax					= 100000.0f;
	xPID.fIMin					= -100000.0f;
	xPID.fDMax					= 100000.0f;
	xPID.fDMin					= -100000.0f;
	xPID.fOutMax				= 100000.0f;
	xPID.fOutMin				= -100000.0f;
	xPID.pfGetSample			= fGetSample;
	xPID.pfUpdate				= vUpdate;
	vHOS_PID_init(&xPID);

	pfYCurrent = &fYCurrent;
	pfSetPoint = &xPID.fSetPoint;
	pfKi = &xPID.fKi;
	pfKp = &xPID.fKp;
	pfKd = &xPID.fKd;
	puiTimeInvervalMs = &xPID.uiTimeIntervalMs;
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

#endif	/*	PID_SIM_EXAMPLE	*/
