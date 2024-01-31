/*
 * Port_DAC.h
 *
 *  Created on: Sep 25, 2023
 *      Author: Ali Emad
 */

/*	Target checking	*/
#include "MCAL_Port/Port_Target.h"
#ifdef MCAL_PORT_TARGET_STM32F401RCT6


#ifndef COTS_OS_INC_MCAL_PORT_PORT_DAC_H_
#define COTS_OS_INC_MCAL_PORT_PORT_DAC_H_

#include "MCAL_Port/Port_DIO.h"

/*******************************************************************************
 * Configuration:
 ******************************************************************************/
#define iCONF_DAC_MIN_VOLTAGE_MV		0
#define iCONF_DAC_MAX_VOLTAGE_MV		3300

/*******************************************************************************
 * API functions / macros:
 ******************************************************************************/
/*
 * Initializes DAC unit.
 */
void vPort_DAC_initUnit(uint8_t ucUnitNumber);

/*
 * Initializes DAC channel.
 *
 * Notes:
 * 		-	Pin mapping must be done independently.
 */
void vPort_DAC_initChannel(uint8_t ucUnitNumber, uint8_t ucChannelNumber);

/*
 * Sets channel voltage.
 */
void vPort_DAC_setChannelVoltage(	uint8_t ucUnitNumber,
									uint8_t ucChannelNumber,
									int32_t iVoltageMV	);

/*
 * Gets DAC resolution.
 *
 * This macro evaluates to the number of steps that could be achieved between 0% and
 * 100% of full analog swing on any DAC channel on the given DAC unit.
 */
#define uiPORT_DAC_GET_RESOLUTION(ucUnitNumber)		\
	(	/*uiPORT_TIM_GET_PWM_RESOLUTION(ucUnitNumber)*/	)

/*
 * Writes DAC word regardless of its reference voltage.
 */
#define vPORT_DAC_WRITE_WORD(ucUnitNumber, ucChannelNumber, ucWord)		\
	(	vPORT_DIO_WRITE_PORT(0, 0xFF, ucWord)	)





#endif /* COTS_OS_INC_MCAL_PORT_PORT_DAC_H_ */


#endif /* Target checking */