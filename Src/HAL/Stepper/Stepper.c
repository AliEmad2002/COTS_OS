/*
 * Stepper.c
 *
 *  Created on: Jun 28, 2023
 *      Author: Ali Emad
 */

/*	LIB	*/
#include <stdint.h>

/*	FreeRTOS	*/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/*	MCAL (Ported)	*/
#include "MCAL_Port/Port_DIO.h"
#include "MCAL_Port/Port_BusyWait1us.h"

/*	SELF	*/
#include "HAL/Stepper/Stepper.h"

/*******************************************************************************
 * API functions:
 ******************************************************************************/
/*
 * See header file for info.
 */
void vHOS_Stepper_init(xHOS_Stepper_t* pxHandle)
{
	vPort_DIO_initPinOutput(pxHandle->ucEnPort, pxHandle->ucEnPin);
	vHOS_Stepper_disable(pxHandle);

	vPort_DIO_initPinOutput(pxHandle->ucStepPort, pxHandle->ucStepPin);
	vPort_DIO_writePin(pxHandle->ucStepPort, pxHandle->ucStepPin, 0);

	vPort_DIO_initPinOutput(pxHandle->ucDirPort, pxHandle->ucDirPin);
	vHOS_Stepper_setDir(pxHandle, ucHOS_STEPPER_DIR_FORWARD);

	pxHandle->iCurrentPos = 0;
}

/*
 * See header file for info.
 */
__attribute__((always_inline)) inline
void vHOS_Stepper_setDir(xHOS_Stepper_t* pxHandle, uint8_t ucDir)
{
	if (ucDir == ucHOS_STEPPER_DIR_FORWARD)
	{
		vPort_DIO_writePin(pxHandle->ucDirPort, pxHandle->ucDirPin, 0);
		pxHandle->cPosIncrementer = 1;
	}

	else
	{
		vPort_DIO_writePin(pxHandle->ucDirPort, pxHandle->ucDirPin, 1);
		pxHandle->cPosIncrementer = -1;
	}
}

/*
 * See header file for info.
 */
__attribute__((always_inline)) inline
void vHOS_Stepper_stepSingle(xHOS_Stepper_t* pxHandle)
{
	vPort_DIO_writePin(pxHandle->ucStepPort, pxHandle->ucStepPin, 1);

	vPort_BusyWait1us();
	vPort_BusyWait1us();

	vPort_DIO_writePin(pxHandle->ucStepPort, pxHandle->ucStepPin, 0);

	pxHandle->iCurrentPos += pxHandle->cPosIncrementer;
}

/*
 * See header file for info.
 */
void vHOS_Stepper_stepN(xHOS_Stepper_t* pxHandle, int32_t iN, int32_t iUsStepInterval)
{
	/*	set direction, and make 'iN' a magnitude	*/
	if(iN >= 0)
		vHOS_Stepper_setDir(pxHandle, ucHOS_STEPPER_DIR_FORWARD);
	else
	{
		vHOS_Stepper_setDir(pxHandle, ucHOS_STEPPER_DIR_BACKWARD);
		iN = -iN;
	}

	/*	Take 'HardwareDelay_t' unit mutex	*/
	xSemaphoreTake(pxHandle->pxHardwareDelayHandle->xMutex, portMAX_DELAY);

	/*	make N steps with the requested delay	*/
	while(iN--)
	{
		vHOS_Stepper_stepSingle(pxHandle);
		vHOS_HardwareDelay_delayUs(pxHandle->pxHardwareDelayHandle, iUsStepInterval);
	}

	/*	Give 'HardwareDelay_t' unit mutex	*/
	xSemaphoreGive(pxHandle->pxHardwareDelayHandle->xMutex);
}

/*
 * See header file for info.
 */
__attribute__((always_inline)) inline
void vHOS_Stepper_enable(xHOS_Stepper_t* pxHandle)
{
	vPort_DIO_writePin(pxHandle->ucEnPort, pxHandle->ucEnPin, 0);
	pxHandle->ucIsEnabled = 1;
}

/*
 * See header file for info.
 */
__attribute__((always_inline)) inline
void vHOS_Stepper_disable(xHOS_Stepper_t* pxHandle)
{
	vPort_DIO_writePin(pxHandle->ucEnPort, pxHandle->ucEnPin, 1);
	pxHandle->ucIsEnabled = 0;
}













