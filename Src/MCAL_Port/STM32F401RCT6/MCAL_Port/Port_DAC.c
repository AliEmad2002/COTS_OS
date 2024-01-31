/*
 * Port_DAC.c
 *
 *  Created on: Sep 25, 2023
 *      Author: Ali Emad
 */

/*	Target checking	*/
#include "MCAL_Port/Port_Target.h"
#ifdef MCAL_PORT_TARGET_STM32F401RCT6


/*	LIB	*/
#include "stdint.h"
#include "LIB/Assert.h"

/*	MCAL	*/
#include "MCAL_Port/Port_DIO.h"

/*	SELF	*/
#include "MCAL_Port/Port_DAC.h"


void vPort_DAC_initUnit(uint8_t ucUnitNumber)
{

}

void vPort_DAC_initChannel(uint8_t ucUnitNumber, uint8_t ucChannelNumber)
{
	for (uint8_t i = 0; i < 8; i++)
		vPort_DIO_initPinOutput(0, i);
}

void vPort_DAC_setChannelVoltage(	uint8_t ucUnitNumber,
									uint8_t ucChannelNumber,
									int32_t iVoltageMV	)
{
	uint8_t ucVal = (iVoltageMV * 255) / 3300;

	vPORT_DIO_WRITE_PORT(0, 0xFF, ucVal);
}


#endif /* Target checking */