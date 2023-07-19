/*
 * Port_GPIO.c
 *
 *  Created on: Jun 13, 2023
 *      Author: Ali Emad
 */

#include "stm32f103xb.h"
#include "stm32f1xx_hal.h"
#include "MCAL_Port/Port_DIO.h"

extern SPI_TypeDef* const pxPortSpiArr[];

/*******************************************************************************
 * Helping structures:
 ******************************************************************************/
typedef struct{
	uint8_t ucNssPort;
	uint8_t ucNssPin;

	uint8_t ucSckPort;
	uint8_t ucSckPin;

	uint8_t ucMISOPort;
	uint8_t ucMISOPin;

	uint8_t ucMOSIPort;
	uint8_t ucMOSIPin;
}xPort_GPIO_SpiMap_t;

typedef struct{
	uint8_t ucSclPort;
	uint8_t ucSclPin;

	uint8_t ucSdaPort;
	uint8_t ucSdaPin;
}xPort_GPIO_I2CMap_t;

/*******************************************************************************
 * Helping functions:
 ******************************************************************************/
static void vPort_GPIO_initPinAFPP(uint8_t ucPort, uint8_t ucPin)
{
	GPIO_InitTypeDef xConf;
	xConf.Pin = 1 << ucPin;
	xConf.Mode = GPIO_MODE_AF_PP;
	xConf.Pull = GPIO_NOPULL;
	xConf.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(pxPortDioPortArr[ucPort], &xConf);
}

static void vPort_GPIO_initPinAFOD(uint8_t ucPort, uint8_t ucPin)
{
	GPIO_InitTypeDef xConf;
	xConf.Pin = 1 << ucPin;
	xConf.Mode = GPIO_MODE_AF_OD;
	xConf.Pull = GPIO_NOPULL;
	xConf.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(pxPortDioPortArr[ucPort], &xConf);
}

/*******************************************************************************
 * API functions:
 ******************************************************************************/
void vPort_GPIO_initSpiPins(	uint8_t ucUnitNumber,
								uint8_t ucMapNumber,
								uint8_t ucInitNSS,
								uint8_t ucInitMISO,
								uint8_t ucInitMOSI	)
{
	const xPort_GPIO_SpiMap_t pxSpi1MapArr[] = {
		{0, 4,  0, 5, 0, 6, 0, 7},
		{0, 15, 1, 3, 1, 4, 1, 5}
	};

	const xPort_GPIO_SpiMap_t xSpi2Map = {1, 12, 1, 13, 1, 14, 1, 15};

	/*	extract data to be used	*/
	const xPort_GPIO_SpiMap_t* pxToUseMap;
	if (ucMapNumber == 0)
		pxToUseMap = &pxSpi1MapArr[ucMapNumber];
	else
		pxToUseMap = &xSpi2Map;

	/*	init NSS pin	*/
	if (ucInitNSS)
	{
		if (READ_BIT(pxPortSpiArr[ucUnitNumber]->CR1, SPI_CR1_MSTR) == 0)
			vPort_DIO_initPinInput(pxToUseMap->ucNssPort, pxToUseMap->ucNssPin, 1);
		else
			vPort_GPIO_initPinAFPP(pxToUseMap->ucNssPort, pxToUseMap->ucNssPin);
	}

	/*	init SCK pin	*/
	vPort_GPIO_initPinAFPP(pxToUseMap->ucSckPort, pxToUseMap->ucSckPin);

	/*	init MISO pin	*/
	if (ucInitMISO)
	{
		vPort_DIO_initPinInput(pxToUseMap->ucMISOPort, pxToUseMap->ucMISOPin, 0);
	}

	/*	init MOSI pin	*/
	if (ucInitMOSI)
		vPort_GPIO_initPinAFPP(pxToUseMap->ucMOSIPort, pxToUseMap->ucMOSIPin);
}

void vPort_GPIO_initI2CPins(uint8_t ucUnitNumber, uint8_t ucMapNumber)
{
	const xPort_GPIO_I2CMap_t pxI2C1Map = {1, 6, 1, 7};
	const xPort_GPIO_I2CMap_t pxI2C2Map = {1, 10, 1, 11};

	/*	extract data to be used	*/
	const xPort_GPIO_I2CMap_t* pxToUseMap;
	if (ucMapNumber == 0)
		pxToUseMap = &pxI2C1Map;
	else
		pxToUseMap = &pxI2C2Map;

	/*	init SCL pin	*/
	vPort_GPIO_initPinAFOD(pxToUseMap->ucSclPort, pxToUseMap->ucSclPin);

	/*	init SDA pin	*/
	vPort_GPIO_initPinAFOD(pxToUseMap->ucSdaPort, pxToUseMap->ucSdaPin);
}
