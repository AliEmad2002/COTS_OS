/*
 * Port_SPI.h
 *
 *  Created on: Jun 12, 2023
 *      Author: Ali Emad
 */

#ifndef HAL_OS_PORT_PORT_SPI_H_
#define HAL_OS_PORT_PORT_SPI_H_

/*
 * SPI driver of target MCU is included here, and its proper functions are called
 * inside the following defined wrapper functions.
 *
 * If such a function is not provided by the available driver, user may write it
 * inside the wrapper function (if too long to be inlined, remove the "inline"
 * identifier).
 *
 * If certain functionality is not even available in the target HW, its wrapper
 * is left empty.
 */
#include "stm32f1xx.h"
#include "cmsis_gcc.h"
#include "Bit_Math.h"
#include "Port/Port_AFIO.h"
#include "Port/Port_GPIO.h"

extern SPI_TypeDef* const pxPortSpiArr[];

/*******************************************************************************
 * Helping structures:
 ******************************************************************************/
typedef struct{
	uint8_t ucFullDuplexEn     : 1;
	uint8_t ucFrameFormat8     : 1;
	uint8_t ucFrameFormat16    : 1;
	uint8_t ucLSBitFirst       : 1;
	uint8_t ucIsMaster         : 1;
	uint8_t ucMOSIEn           : 1;
	uint8_t ucMISOEn           : 1;
	uint8_t ucNssEn            : 1;
	uint8_t ucAFIOMapNumber;
	uint8_t ucComMode;
	uint16_t usBaudratePrescaler;
}xPort_SPI_HW_Conf_t;

/*******************************************************************************
 * Functions:
 ******************************************************************************/
/*
 * Enables unidirectional mode.
 * i.e: makes communication via two lines (full-duplex).
 */
static inline void vPort_SPI_setFullDuplex(uint8_t ucUnitNumber)
{
	//SPI_SET_FULL_DUPLEX(ucUnitNumber);
	CLEAR_BIT(pxPortSpiArr[ucUnitNumber]->CR1, SPI_CR1_BIDIMODE);
}

/*
 * Sets frame-format to 8-bits.
 */
static inline void vPort_SPI_setFrameFormat8Bit(uint8_t ucUnitNumber)
{
	//SPI_SET_FRAME_FORMAT_8_BIT_NO_WAIT(ucUnitNumber);
	CLEAR_BIT(pxPortSpiArr[ucUnitNumber]->CR1, SPI_CR1_DFF);
}

/*
 * Sets frame-format to 16-bits.
 */
static inline void vPort_SPI_setFrameFormat16Bit(uint8_t ucUnitNumber)
{
	//SPI_SET_FRAME_FORMAT_16_BIT_NO_WAIT(ucUnitNumber);
	SET_BIT(pxPortSpiArr[ucUnitNumber]->CR1, SPI_CR1_DFF);
}

/*
 * Sets frame direction to MSB_First.
 */
static inline void vPort_SPI_setMSBFirst(uint8_t ucUnitNumber)
{
	//SPI_SET_MSB_FIRST(ucUnitNumber);
	CLEAR_BIT(pxPortSpiArr[ucUnitNumber]->CR1, SPI_CR1_LSBFIRST);
}


/*
 * Sets frame direction to LSB_First.
 */
static inline void vPort_SPI_setLSBFirst(uint8_t ucUnitNumber)
{
	//SPI_SET_LSB_FIRST(ucUnitNumber);
	SET_BIT(pxPortSpiArr[ucUnitNumber]->CR1, SPI_CR1_LSBFIRST);
}

/*
 * Sets baudrate prescaler.
 */
static inline void vPort_SPI_setBaudratePrescaler(uint8_t ucUnitNumber, uint16_t usPrescaler)
{
	//SPI_voidSetBaudRatePrescalerNumericNoWait(ucUnitNumber, usPrescaler);
	switch(usPrescaler)
	{
	case 2:
		vEDT_REG(pxPortSpiArr[ucUnitNumber]->CR1, 3, 0, 3);
		break;

	case 4:
		vEDT_REG(pxPortSpiArr[ucUnitNumber]->CR1, 3, 1, 3);
		break;

	case 8:
		vEDT_REG(pxPortSpiArr[ucUnitNumber]->CR1, 3, 2, 3);
		break;

	case 16:
		vEDT_REG(pxPortSpiArr[ucUnitNumber]->CR1, 3, 3, 3);
		break;

	case 32:
		vEDT_REG(pxPortSpiArr[ucUnitNumber]->CR1, 3, 4, 3);
		break;

	case 64:
		vEDT_REG(pxPortSpiArr[ucUnitNumber]->CR1, 3, 5, 3);
		break;

	case 128:
		vEDT_REG(pxPortSpiArr[ucUnitNumber]->CR1, 3, 6, 3);
		break;

	case 256:
		vEDT_REG(pxPortSpiArr[ucUnitNumber]->CR1, 3, 7, 3);
		break;
	default:
		configASSERT(0);
	}
}

/*
 * Selects the SPI unit as a master.
 */
static inline void vPort_SPI_enableMasterMode(uint8_t ucUnitNumber)
{
	//SPI_ENABLE_MASTER_MODE(ucUnitNumber);
	/*	SW slave management enable	*/
	SET_BIT(pxPortSpiArr[ucUnitNumber]->CR1, SPI_CR1_SSM);
	/*	SW slave management deselect (self deselect)	*/
	SET_BIT(pxPortSpiArr[ucUnitNumber]->CR1, SPI_CR1_SSI);
	/*	Select master mode	*/
	SET_BIT(pxPortSpiArr[ucUnitNumber]->CR1, SPI_CR1_MSTR);
}

/*
 * Selects SPI mode.
 *
 * Notice:
 *         Mode     clock polarity (idle on:)   clock phase (capture first/second edge)
 *           0                 0                                  first
 *           1                 0                                  second
 *           2                 1                                  first
 *           3                 1                                  second
 */
static inline void vPort_SPI_setComMode(uint8_t ucUnitNumber, uint8_t ucMode)
{
	//SPI_SET_COM_MODE(ucUnitNumber, ucMode);
	vEDT_REG(pxPortSpiArr[ucUnitNumber]->CR1, 0, ucMode, 2);
}

///*
// * Initializes GPIO pins which correspond to the SPI unit.
// *
// * Notes:
// * 		-	user may omit (ignore) all/any of the last 4 arguments if they are of no use
// * 		in their port target.
// */
//static inline void vPort_SPI_initGPIO(	uint8_t ucUnitNumber,
//										uint8_t ucMap,		/*	AFIO map number	*/
//										uint8_t icInitNSS,	/*	set to 1 if NSS pin is needed	*/
//										uint8_t icInitMISO,	/*	set to 1 if MISO pin is needed	*/
//										uint8_t icInitMOSI	/*	set to 1 if MOSI pin is needed	*/
//										)
//{
//	SPI_voidInitPins(ucUnitNumber, ucMap, icInitNSS, icInitMISO, icInitMOSI);
//}

/*
 * Enables SPI unit.
 */
static inline void vPort_SPI_enable(uint8_t ucUnitNumber)
{
	//SPI_ENABLE_PERIPHERAL(ucUnitNumber);
	SET_BIT(pxPortSpiArr[ucUnitNumber]->CR1, SPI_CR1_SPE);
}


/*
 * Disables SPI unit.
 */
static inline void vPort_SPI_disable(uint8_t ucUnitNumber)
{
	//SPI_DISABLE_PERIPHERAL(ucUnitNumber);
	CLEAR_BIT(pxPortSpiArr[ucUnitNumber]->CR1, SPI_CR1_SPE);
}

/*
 * Enables transfer complete interrupt.
 */
#define vPORT_SPI_ENABLE_TXC_INTERRUPT(ucUnitNumber)	\
	(SET_BIT(pxPortSpiArr[(ucUnitNumber)]->CR2, SPI_CR2_RXNEIE))

/*
 * Disables transfer complete interrupt.
 */
#define vPORT_SPI_DISABLE_TXC_INTERRUPT(ucUnitNumber)	\
	(CLEAR_BIT(pxPortSpiArr[(ucUnitNumber)]->CR2, SPI_CR2_RXNEIE))

/*
 * Clears transfer complete flag.
 */
#define vPORT_SPI_CLEAR_TXC_FLAG(ucUnitNumber)	\
	((volatile void)(pxPortSpiArr[(ucUnitNumber)]->DR))

/*
 * Checks if SPI unit is busy.
 */
#define ucPort_SPI_IS_BUSY(ucUnitNumber)	(ucGET_BIT(pxPortSpiArr[(ucUnitNumber)]->SR, 7))

/*
 * Writes a byte on the data register. (no waiting).
 */
#define vPort_SPI_WRT_DR_NO_WAIT(ucUnitNumber, ucByte)	\
		(pxPortSpiArr[(ucUnitNumber)]->DR = (ucByte))


/*
 * Reads the byte on the data register. (no waiting).
 */
#define vPort_SPI_GET_DR_NO_WAIT(ucUnitNumber)	\
		(pxPortSpiArr[(ucUnitNumber)]->DR)

/*
 * Initializes HW of an SPI unit.
 *
 * Notes:
 * 		-	This function must be called on startup (inside "vPort_HW_init()")
 * 			for all units that are going to be used.
 */
static inline void vPort_SPI_initHardware(uint8_t ucUnitNumber, xPort_SPI_HW_Conf_t* pxHWConf)
{
	if (pxHWConf->ucFullDuplexEn)
		vPort_SPI_setFullDuplex(ucUnitNumber);

	if (pxHWConf->ucFrameFormat8)
		vPort_SPI_setFrameFormat8Bit(ucUnitNumber);
	else
		vPort_SPI_setFrameFormat16Bit(ucUnitNumber);

	if (pxHWConf->ucLSBitFirst)
		vPort_SPI_setLSBFirst(ucUnitNumber);
	else
		vPort_SPI_setMSBFirst(ucUnitNumber);

	vPort_SPI_setBaudratePrescaler(ucUnitNumber, pxHWConf->usBaudratePrescaler);

	if (pxHWConf->ucIsMaster)
		vPort_SPI_enableMasterMode(ucUnitNumber);

	vPort_SPI_setComMode(ucUnitNumber, pxHWConf->ucComMode);

	vPort_SPI_enable(ucUnitNumber);

	vPort_AFIO_mapSpi(ucUnitNumber, pxHWConf->ucAFIOMapNumber);

	vPort_GPIO_initSpiPins(	ucUnitNumber,
							pxHWConf->ucAFIOMapNumber,
							pxHWConf->ucNssEn,
							pxHWConf->ucMISOEn,
							pxHWConf->ucMOSIEn	);
}

/*******************************************************************************
 * Handlers (Vectors):
 ******************************************************************************/
/*
 * Define SPI transfer complete handlers.
 *
 * Notes:
 * 		-	If the target MCU does not have a unique SPI_TC vector, user may define
 * 			SPI_Global vector. As anyways, to avoid this problem, upper layer drivers
 * 			use flag checking macros in ISR beginning.
 *
 * 		-	In order for the driver "SPI.c" to be able to use these handlers,
 * 		they must be defined like the following example:
 * 			#define configHOS_SPI_HANDLER_0		SPI1_IRQHandler
 * 			#define configHOS_SPI_HANDLER_1		SPI2_IRQHandler
 * 			... And so on.
 */
#define port_SPI_HANDLER_0		SPI1_IRQHandler











#endif /* HAL_OS_PORT_PORT_SPI_H_ */
