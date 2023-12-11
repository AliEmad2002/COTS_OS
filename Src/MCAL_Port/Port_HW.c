/*
 * Port_HW.c
 *
 *  Created on: Sep 25, 2023
 *      Author: Ali Emad
 */


#include "stdint.h"

#include "MCAL_Port/Port_HW.h"

#include "MCAL_Port/Port_Clock.h"
#include "MCAL_Port/Port_PWR.h"
#include "MCAL_Port/Port_BKP.h"
#include "MCAL_Port/Port_Interrupt.h"
#include "MCAL_Port/Port_RTC.h"
#include "MCAL_Port/Port_USB.h"
#include "MCAL_Port/Port_UART.h"
#include "MCAL_Port/Port_AFIO.h"
#include "MCAL_Port/Port_GPIO.h"
#include "MCAL_Port/Port_ADC.h"
#include "MCAL_Port/Port_I2C.h"
#include "MCAL_Port/Port_SPI.h"

static void vInitUartUnit(uint8_t i)
{
	static const uint8_t pucMapArr[] = {0, 0, 0};
	vPort_UART_initHW(i);
	vPort_UART_setTransferDirection(i, 2);
	vPort_UART_setStopBitsLength(i, 1);
	vPort_UART_setBaudRate(i, 9600);
	vPort_AFIO_mapUart(i, pucMapArr[i]);
	vPort_GPIO_initUartPins(i, pucMapArr[i], 0, 1);
}

void vPort_HW_init(void)
{
	/*	Initialize peripherals' clocks	*/
	vPort_Clock_initPeriphClock();

	/*	Initialize PWR	*/
	vPort_PWR_init();

	/*	Initialize BKP	*/
	vPort_BKP_init();

	/*	Initialize RTC	*/
	vPort_RTC_init();
	vPort_RTC_enable();

	/*	Initialize UART		*/
	vInitUartUnit(0);
	vInitUartUnit(1);
	vInitUartUnit(2);

	/*	Initialize SPI 		*/
	xPort_SPI_HW_Conf_t xSpiConf = {
		.ucFullDuplexEn         = 1,
		.ucLSBitFirst           = 0,
		.ucIsMaster             = 1,
		.ucMOSIEn               = 1,
		.ucMISOEn               = 1,
		.ucNssEn                = 0,
		.ucComMode              = 0,
		.usBaudratePrescaler    = 256,
		.ucAFIOMapNumber		= 0
	};
	vPort_SPI_initHardware(0, &xSpiConf);

	/*	Initialize I2C HW	*/
	xPort_I2C_HW_Conf_t xI2cConf = {
		.ucAFIOMapNumber = 0,
		.ucClockMode = 0,
		.ucIsAddress7Bit = 1,
		.uiSclFrequencyHz = 100000,
		.uiMaxRisingTimeNs = 1000
	};
	vPort_I2C_initHardware(0, &xI2cConf);

	/*	Initialize USB	*/
	vPort_USB_initHardware();
}
