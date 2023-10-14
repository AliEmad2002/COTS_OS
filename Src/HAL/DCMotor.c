/*
 * DCMotor.c
 *
 *  Created on: Aug 31, 2023
 *      Author: Ali Emad
 */

/*	LIB	*/
#include <stdint.h>

/*	FreeRTOS	*/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/*	MCAL (Ported)	*/
#include "MCAL_Port/Port_Timer.h"
#include "MCAL_Port/Port_DIO.h"
#include "MCAL_Port/Port_GPIO.h"

/*	SELF	*/
#include "HAL/DCMotor/DCMotor.h"



/*
 * See header for info.
 */
void vHOS_DCMotor_init(xHOS_DCMotor_t* pxHandle)
{
	/*	Initialize timer channel	*/
	vPort_TIM_initChannelPwmOutput(	pxHandle->ucDir1TimerUnitNumber,
									pxHandle->ucDir1TimerChannelNumber	);

	/*	Initialize pins	*/
	vPort_GPIO_initTimerChannelPinAsOutput(	pxHandle->ucDir1TimerUnitNumber,
											pxHandle->ucDir1TimerChannelNumber,
											0	);

	vPort_DIO_initPinOutput(pxHandle->ucEnPort, pxHandle->ucEnPin);

	vPort_DIO_initPinOutput(pxHandle->ucDir2Port, pxHandle->ucDir2Pin);

	/*	Motor is initially disabled	*/
	vHOS_DCMOTOR_DISABLE(pxHandle);

	/*	Speed is initially zero	*/
	vHOS_DCMotor_setSpeed(pxHandle, 0);
}

/*
 * See header for info.
 */
void vHOS_DCMotor_setSpeed(xHOS_DCMotor_t* pxHandle, int32_t iSpeed)
{
	uint16_t usDuty;

	taskENTER_CRITICAL();
	{
		if (iSpeed >= 0)
		{
			usDuty = iSpeed;
			vPORT_DIO_WRITE_PIN(pxHandle->ucDir2Port, pxHandle->ucDir2Pin, 0);
		}

		else
		{
			usDuty = 65535 + iSpeed;
			vPORT_DIO_WRITE_PIN(pxHandle->ucDir2Port, pxHandle->ucDir2Pin, 1);
		}

		vPort_TIM_setPwmDuty(	pxHandle->ucDir1TimerUnitNumber,
								pxHandle->ucDir1TimerChannelNumber,
								(uint16_t)usDuty	);
	}
	taskEXIT_CRITICAL();
}
