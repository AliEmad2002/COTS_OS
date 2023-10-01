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

/*	SELF	*/
#include "MCAL_Port/Port_DAC.h"


void vPort_DAC_initUnit(uint8_t ucUnitNumber)
{
	/*	Set PWM frequency	*/
	vLib_ASSERT(uiPort_TIM_setOvfFreq(ucUnitNumber, uiCONF_DAC_PWM_FREQ_HZ), 0);
}

void vPort_DAC_initChannel(uint8_t ucUnitNumber, uint8_t ucChannelNumber)
{
	/*	Initialize PWM output	*/
	vPort_TIM_initChannelPwmOutput(ucUnitNumber, ucChannelNumber);
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
