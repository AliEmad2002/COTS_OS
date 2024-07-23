/*
 * Port_GPIO.c
 *
 *  Created on: Jun 13, 2023
 *      Author: Ali Emad
 */

/*	Target checking	*/
#include "MCAL_Port/Port_Target.h"
#ifdef MCAL_PORT_TARGET_STM32F401RCT6

#include "LIB/Assert.h"

#include "stm32f401xc.h"
#include "stm32f4xx_hal.h"
#include "MCAL_Port/Port_DIO.h"


/*******************************************************************************
 * Exetrns:
 ******************************************************************************/
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

typedef struct{
	uint8_t ucSclPort : 4;
	uint8_t ucSclPin : 4;

	uint8_t ucSdaPort : 4;
	uint8_t ucSdaPin : 4;
}xPort_GPIO_I2CMap_t;

typedef struct{
	uint8_t ucTxPort : 4;
	uint8_t ucTxPin : 4;

	uint8_t ucRxPort : 4;
	uint8_t ucRxPin : 4;
}xPort_GPIO_UartMap_t;

typedef struct{
	uint8_t ucPort : 4;
	uint8_t ucPin : 4;
}xPort_GPIO_AdcMap_t;

typedef struct{
	uint8_t ucPort : 4;
	uint8_t ucPin : 4;
	uint8_t ucAf;
}xPort_GPIO_TimerMap_t;


/*******************************************************************************
 * Private (Static) variables:
 ******************************************************************************/
static const xPort_GPIO_SpiMap_t pxSpiMapArr[] = {
	{0, 0, 0, 5, 1, 4, 0, 7},
	{0, 0, 1, 13, 1, 14, 1, 15}
};

static const xPort_GPIO_UartMap_t pxUartMapArr[] = {
	{0, 9, 0, 10},
	{0, 2, 0, 3}
};

static const xPort_GPIO_I2CMap_t pxI2CMapArr[] = {
	{1, 8, 1, 9},
	{1, 10, 1, 11}
};

static const xPort_GPIO_AdcMap_t pxAdcMapArr[] = {
	{0, 0}, {0, 1}, {0, 2}, {0, 3},
	{0, 4}, {0, 5}, {0, 6}, {0, 7},
	{1, 0}, {1, 1}
};

static const xPort_GPIO_TimerMap_t pxTimer0MapArr[] = {
	/*	TODO	*/
};

static const xPort_GPIO_TimerMap_t pxTimer1MapArr[] = {
	{0, 5, GPIO_AF1_TIM2}, {0, 1, GPIO_AF1_TIM2},
	{1, 10, GPIO_AF1_TIM2}, {1, 11, GPIO_AF1_TIM2}
};

static const xPort_GPIO_TimerMap_t pxTimer2MapArr[] = {
	/*	TODO	*/
};

static const xPort_GPIO_TimerMap_t pxTimer3MapArr[] = {
	{0, 5, GPIO_AF2_TIM4}, {0, 1, GPIO_AF2_TIM4},
	{1, 8, GPIO_AF2_TIM4}, {1, 11, GPIO_AF2_TIM4}
};

static const xPort_GPIO_TimerMap_t ppxTimerMapArr[] = {
	pxTimer0MapArr, pxTimer1MapArr, pxTimer2MapArr, pxTimer3MapArr,
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

static void vPort_GPIO_initPinAFPPPD(uint8_t ucPort, uint8_t ucPin, uint32_t uiAF)
{
	GPIO_InitTypeDef xConf;
	xConf.Pin = 1ul << ucPin;
	xConf.Mode = GPIO_MODE_AF_PP;
	xConf.Pull = GPIO_PULLDOWN;
	xConf.Speed = GPIO_SPEED_FREQ_HIGH;
	xConf.Alternate = uiAF;
	HAL_GPIO_Init(pxPortDioPortArr[ucPort], &xConf);
}

static void vPort_GPIO_initPinAFOD(uint8_t ucPort, uint8_t ucPin, uint32_t uiAF)
{
	GPIO_InitTypeDef xConf;
	xConf.Pin = 1ul << ucPin;
	xConf.Mode = GPIO_MODE_AF_OD;
	xConf.Pull = GPIO_NOPULL;
	xConf.Speed = GPIO_SPEED_FREQ_HIGH;
	xConf.Alternate = uiAF;
	HAL_GPIO_Init(pxPortDioPortArr[ucPort], &xConf);
}

static void vPort_GPIO_initPinAFODPU(uint8_t ucPort, uint8_t ucPin, uint32_t uiAF)
{
	GPIO_InitTypeDef xConf;
	xConf.Pin = 1ul << ucPin;
	xConf.Mode = GPIO_MODE_AF_OD;
	xConf.Pull = GPIO_PULLUP;
	xConf.Speed = GPIO_SPEED_FREQ_HIGH;
	xConf.Alternate = uiAF;
	HAL_GPIO_Init(pxPortDioPortArr[ucPort], &xConf);
}

static void vPort_GPIO_initPinAnalogIn(uint8_t ucPort, uint8_t ucPin)
{
	GPIO_InitTypeDef xConf;
	xConf.Pin = 1ul << ucPin;
	xConf.Mode = GPIO_MODE_ANALOG;
	xConf.Pull = GPIO_NOPULL;
	xConf.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(pxPortDioPortArr[ucPort], &xConf);
}

/*******************************************************************************
 * API functions:
 ******************************************************************************/
/*
 * See header for info
 */
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
		vPort_GPIO_initPinAFPPPU(pxMap->ucMISOPort, pxMap->ucMISOPin, GPIO_AF5_SPI1);
	}

	/*	init MOSI pin	*/
	if (ucInitMOSI)
	{
		vPort_GPIO_initPinAFPP(pxMap->ucMOSIPort, pxMap->ucMOSIPin, GPIO_AF5_SPI1);
	}
}

/*
 * See header for info
 */
void vPort_GPIO_initUartPins(	uint8_t ucUnitNumber,
								uint8_t ucInitTx,
								uint8_t ucInitRx	)
{
	uint8_t ucPort, ucPin;

	/*	Init Tx	*/
	if (ucInitTx)
	{
		ucPort = pxUartMapArr[ucUnitNumber].ucTxPort;
		ucPin = pxUartMapArr[ucUnitNumber].ucTxPin;
		vPort_GPIO_initPinAFPP(ucPort, ucPin, GPIO_AF7_USART1);
	}

	/*	Init Rx	*/
	if (ucInitRx)
	{
		ucPort = pxUartMapArr[ucUnitNumber].ucRxPort;
		ucPin = pxUartMapArr[ucUnitNumber].ucRxPin;
		vPort_GPIO_initPinAFPP(ucPort, ucPin, GPIO_AF7_USART1);
	}
}

/*
 * See header for info
 */
void vPort_GPIO_deInitUartPins(	uint8_t ucUnitNumber,
								uint8_t ucDeInitTx,
								uint8_t ucDeInitRx	)
{
	uint8_t ucPort, ucPin;

	/*	De-init Tx	*/
	if (ucDeInitTx)
	{
		ucPort = pxUartMapArr[ucUnitNumber].ucTxPort;
		ucPin = pxUartMapArr[ucUnitNumber].ucTxPin;
		vPort_DIO_initPinInput(ucPort, ucPin, 0);
	}

	/*	Init Rx	*/
	if (ucDeInitRx)
	{
		ucPort = pxUartMapArr[ucUnitNumber].ucRxPort;
		ucPin = pxUartMapArr[ucUnitNumber].ucRxPin;
		vPort_DIO_initPinInput(ucPort, ucPin, 0);
	}
}

/*
 * See header for info
 */
void vPort_GPIO_initI2cPins(uint8_t ucUnitNumber)
{
	/*	init SCL pin	*/
	vPort_GPIO_initPinAFODPU(
			pxI2CMapArr[ucUnitNumber].ucSclPort,
			pxI2CMapArr[ucUnitNumber].ucSclPin,
			GPIO_AF4_I2C1	);

	/*	init SDA pin	*/
	vPort_GPIO_initPinAFODPU(
			pxI2CMapArr[ucUnitNumber].ucSdaPort,
			pxI2CMapArr[ucUnitNumber].ucSdaPin,
			GPIO_AF4_I2C1	);
}

/*
 * See header for info
 */
void vPort_GPIO_initTimChannelPin(	uint8_t ucUnitNumber,
									uint8_t ucChannelNumber	)
{
	const xPort_GPIO_TimerMap_t* pxMap = ppxTimerMapArr[ucUnitNumber];
	uint8_t ucPort = pxMap[ucChannelNumber].ucPort;
	uint8_t ucPin = pxMap[ucChannelNumber].ucPin;
	uint8_t ucAf = pxMap[ucChannelNumber].ucAf;

	vPort_GPIO_initPinAFPP(ucPort, ucPin, ucAf);
}

/*
 * See header for info
 */
void vPort_GPIO_initAdcChannelPinAsInput(	uint8_t ucAdcUnitNumber,
											uint8_t ucAdcChannelNumber	)
{
	vLib_ASSERT(ucAdcChannelNumber <= 9, 0);

	uint8_t ucPin = pxAdcMapArr[ucAdcChannelNumber].ucPin;
	uint8_t ucPort = pxAdcMapArr[ucAdcChannelNumber].ucPort;

	vPort_GPIO_initPinAnalogIn(ucPort, ucPin);
}

#endif /* Target checking */
