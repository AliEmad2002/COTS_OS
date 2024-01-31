/*
 * Port_GPIO.c
 *
 *  Created on: Jun 13, 2023
 *      Author: Ali Emad
 */

/*	Target checking	*/
#include "MCAL_Port/Port_Target.h"
#ifdef MCAL_PORT_TARGET_STM32F401RCT6


#include "stm32f401xc.h"
#include "stm32f4xx_hal.h"
#include "MCAL_Port/Port_DIO.h"

extern SPI_TypeDef* const pxPortSpiArr[];


/*******************************************************************************
 * Helping structures:
 ******************************************************************************/
/*	SPI mapping	*/
typedef struct{
	uint8_t ucNssPort : 4;
	uint8_t ucNssPin : 4;

	uint8_t ucSckPort : 4;
	uint8_t ucSckPin : 4;

	uint8_t ucMISOPort : 4;
	uint8_t ucMISOPin : 4;

	uint8_t ucMOSIPort : 4;
	uint8_t ucMOSIPin : 4;
}xPort_GPIO_SpiMap_t;

static const xPort_GPIO_SpiMap_t pxSpiMapArr[] = {
	{0, 0, 1, 3, 1, 4, 1, 5}
};

/*******************************************************************************
 * Helping functions:
 ******************************************************************************/
static void vPort_GPIO_initPinAFPP(uint8_t ucPort, uint8_t ucPin, uint32_t uiAF)
{
	GPIO_InitTypeDef xConf;
	xConf.Pin = 1ul << ucPin;
	xConf.Mode = GPIO_MODE_AF_PP;
	xConf.Pull = GPIO_NOPULL;
	xConf.Speed = GPIO_SPEED_FREQ_HIGH;
	xConf.Alternate = uiAF;
	HAL_GPIO_Init(pxPortDioPortArr[ucPort], &xConf);
}

static void vPort_GPIO_initPinAFPPPU(uint8_t ucPort, uint8_t ucPin, uint32_t uiAF)
{
	GPIO_InitTypeDef xConf;
	xConf.Pin = 1ul << ucPin;
	xConf.Mode = GPIO_MODE_AF_PP;
	xConf.Pull = GPIO_PULLUP;
	xConf.Speed = GPIO_SPEED_FREQ_HIGH;
	xConf.Alternate = uiAF;
	HAL_GPIO_Init(pxPortDioPortArr[ucPort], &xConf);
}

static void vPort_GPIO_initPinAFOD(uint8_t ucPort, uint8_t ucPin)
{
	GPIO_InitTypeDef xConf;
	xConf.Pin = 1ul << ucPin;
	xConf.Mode = GPIO_MODE_AF_OD;
	xConf.Pull = GPIO_NOPULL;
	xConf.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(pxPortDioPortArr[ucPort], &xConf);
}

/*******************************************************************************
 * API functions:
 ******************************************************************************/
void vPort_GPIO_initSpiPins(	uint8_t ucUnitNumber,
								uint8_t ucInitNSS,
								uint8_t ucInitMISO,
								uint8_t ucInitMOSI	)
{
	const xPort_GPIO_SpiMap_t* pxMap = &(pxSpiMapArr[ucUnitNumber]);

	/*	init NSS pin	*/
	if (ucInitNSS)
	{
		if (READ_BIT(pxPortSpiArr[ucUnitNumber]->CR1, SPI_CR1_MSTR) == 0)
			vPort_DIO_initPinInput(pxMap->ucNssPort, pxMap->ucNssPin, 1);
		else
			vPort_GPIO_initPinAFPP(pxMap->ucNssPort, pxMap->ucNssPin, GPIO_AF5_SPI1);
	}

	/*	init SCK pin	*/
	vPort_GPIO_initPinAFPP(pxMap->ucSckPort, pxMap->ucSckPin, GPIO_AF5_SPI1);

	/*	init MISO pin	*/
	if (ucInitMISO)
	{
		vPort_GPIO_initPinAFPP(pxMap->ucMISOPort, pxMap->ucMISOPin, GPIO_AF5_SPI1);
	}

	/*	init MOSI pin	*/
	if (ucInitMOSI)
	{
		vPort_GPIO_initPinAFPP(pxMap->ucMOSIPort, pxMap->ucMOSIPin, GPIO_AF5_SPI1);
	}
}


void vPort_GPIO_initTimChannelPin(	uint8_t ucUnitNumber,
									uint8_t ucChannelNumber	)
{
	vPort_GPIO_initPinAFPP(0, 8, GPIO_AF1_TIM1);
}


#endif /* Target checking */