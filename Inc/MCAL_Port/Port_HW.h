/*
 * Port_HW.h
 *
 *  Created on: Jun 17, 2023
 *      Author: Ali Emad
 */

#ifndef HAL_OS_PORT_PORT_HW_H_
#define HAL_OS_PORT_PORT_HW_H_

#include "MCAL_Port/Port_Clock.h"
#include "MCAL_Port/Port_SPI.h"
#include "MCAL_Port/Port_I2C.h"


/*
 * This function is called on system startup. It initializes low level HW (MCAL).
 */
void vPort_HW_init(void)
{
	/*	Clock control	*/
	vPort_Clock_init();

	/*	SPI	*/
//	xPort_SPI_HW_Conf_t xSpi0Conf = {
//		.ucAFIOMapNumber = 0,
//		.ucComMode = 3,
//		.ucFullDuplexEn = 1,
//		.ucIsMaster = 1,
//		.ucLSBitFirst = 0,
//		.ucMISOEn = 0,
//		.ucMOSIEn = 1,
//		.ucNssEn = 0,
//		.usBaudratePrescaler = 2
//	};
//	vPort_SPI_initHardware(0, &xSpi0Conf);

	/*	I2C	*/
	xPort_I2C_HW_Conf_t xI2c0Conf = {
		.ucAFIOMapNumber = 0,
		.ucEnableGeneralCall = 0,
		.ucEnableClockStretching = 0,
		.ucClockMode = 1,
		.ucIsAddress7Bit = 1,
		.usSelfAddress = 0x55, // foo value.
		.uiSclFrequencyHz = 400000,
		.uiMaxRisingTimeNs = 1000
	};

	vPort_I2C_initHardware(0, &xI2c0Conf);
}













#endif /* HAL_OS_PORT_PORT_HW_H_ */
