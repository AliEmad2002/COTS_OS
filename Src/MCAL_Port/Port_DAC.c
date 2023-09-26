/*
 * Port_DAC.c
 *
 *  Created on: Sep 25, 2023
 *      Author: Ali Emad
 */

/*	LIB	*/
#include "stdint.h"
#include "LIB/Assert.h"

/*	MCAL	*/
#include "MCAL_Port/Port_Timer.h"
#include "MCAL_Port/Port_GPIO.h"

/*	SELF	*/
#include "MCAL_Port/Port_DAC.h"


void vPort_DAC_initUnit(uint8_t ucUnitNumber)
{
	/*	Set PWM frequency	*/
	vLib_ASSERT(uiPort_TIM_setOvfFreq(ucUnitNumber, uiCONF_DAC_PWM_FREQ_HZ), 0);

	/*	Initialize all channels of unit's associated timer unit as PWM outputs	*/
	for (uint8_t i = 0; i < 4; i++)
		vPort_TIM_initChannelPwmOutput(ucUnitNumber, i);
}

void vPort_DAC_initChannel(uint8_t ucUnitNumber, uint8_t ucChannelNumber)
{
	/*	Configure timer's channel connection to GPIO pins	*/
	vPort_GPIO_initTimerChannelPinAsOutput(ucUnitNumber, ucChannelNumber, 0);
}

void vPort_DAC_setChannelVoltage(	uint8_t ucUnitNumber,
									uint8_t ucChannelNumber,
									int32_t iVoltageMV	)
{
	uint16_t usDuty =
		(65535l * (iVoltageMV - iCONF_DAC_MIN_VOLTAGE_MV)) /
		(iCONF_DAC_MAX_VOLTAGE_MV - iCONF_DAC_MIN_VOLTAGE_MV);


	vPort_TIM_setPwmDuty(ucUnitNumber, ucChannelNumber, usDuty);
}
