/*
 * Port_HW.c
 *
 *  Created on: Sep 25, 2023
 *      Author: Ali Emad
 */

/*	Target checking	*/
#include "MCAL_Port/Port_Target.h"
#ifdef MCAL_PORT_TARGET_STM32F401RCT6



#include "stdint.h"

#include "MCAL_Port/Port_HW.h"

#include "MCAL_Port/Port_Clock.h"
#include "MCAL_Port/Port_Interrupt.h"
#include "MCAL_Port/Port_GPIO.h"
#include "MCAL_Port/Port_SPI.h"
#include "MCAL_Port/Port_Timer.h"
#include "MCAL_Port/Port_DIO.h"
#include "MCAL_Port/Port_DAC.h"


void vPort_HW_init(void)
{
	/*	Initialize peripherals' clocks	*/
	vPort_Clock_initPeriphClock();

	/*	Initialize SPI	*/
	xPort_SPI_HW_Conf_t xSpiConf = {
		.ucFullDuplexEn         = 1,
		.ucLSBitFirst           = 0,
		.ucIsMaster             = 1,
		.ucMOSIEn               = 1,
		.ucMISOEn               = 1,
		.ucNssEn                = 0,
		.ucComMode              = 0,
		.usBaudratePrescaler    = 256
	};
	vPort_SPI_initHardware(0, &xSpiConf);

//	/*	Initialize DAC	*/
	vPort_DAC_initUnit(0);
	vPort_DAC_initChannel(0, 0);
}


#endif /* Target checking */