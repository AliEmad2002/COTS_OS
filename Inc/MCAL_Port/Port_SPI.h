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
 *
 * For standardization, port SPI driver will only work on 8-bit transmissions.
 */
#include "stm32f1xx.h"
#include "stm32f1xx_ll_spi.h"
#include "MCAL_Port/Port_AFIO.h"
#include "MCAL_Port/Port_GPIO.h"
#include "LIB/Assert.h"

/*******************************************************************************
 * Macros:
 ******************************************************************************/
/*
 * Total number of SPI units in the used target.
 */
#define portSPI_NUMBER_OF_UNITS				2

/*
 * Mapping state between SPI units and DMA (if there's a DMA).
 *
 * 0==> Dynamic mapping.
 * 1==> Static mapping. (Requires configuring the "ppucPortSpiTxeDmaMapping[]" and
 * "ppucPortSpiRxneDmaMapping[]")
 */
#define portSPI_IS_DMA_STATIC_CONNECTED		1

/*******************************************************************************
 * Externs:
 ******************************************************************************/
extern SPI_TypeDef* const pxPortSpiArr[portSPI_NUMBER_OF_UNITS];

extern void (*ppfPortSpiTxeCallbackArr[portSPI_NUMBER_OF_UNITS])(void*);
extern void* ppvPortSpiTxeCallbackParamsArr[portSPI_NUMBER_OF_UNITS];

extern void (*ppfPortSpiRxneCallbackArr[portSPI_NUMBER_OF_UNITS])(void*);
extern void* ppvPortSpiRxneCallbackParamsArr[portSPI_NUMBER_OF_UNITS];

extern const uint8_t ppucPortSpiTxeDmaMapping[portSPI_NUMBER_OF_UNITS][2];

/*******************************************************************************
 * Helping structures:
 ******************************************************************************/
typedef struct{
	uint8_t ucFullDuplexEn     : 1;
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
	LL_SPI_SetTransferDirection(pxPortSpiArr[ucUnitNumber], LL_SPI_FULL_DUPLEX);
}

/*
 * Sets frame-format to 8-bits.
 */
static inline void vPort_SPI_setFrameFormat8Bit(uint8_t ucUnitNumber)
{
	LL_SPI_SetDataWidth(pxPortSpiArr[ucUnitNumber],  LL_SPI_DATAWIDTH_8BIT);
}

/*
 * Sets frame direction to MSB_First.
 */
static inline void vPort_SPI_setMSBFirst(uint8_t ucUnitNumber)
{
	LL_SPI_SetTransferBitOrder(pxPortSpiArr[ucUnitNumber], LL_SPI_MSB_FIRST);
}


/*
 * Sets frame direction to LSB_First.
 */
static inline void vPort_SPI_setLSBFirst(uint8_t ucUnitNumber)
{
	LL_SPI_SetTransferBitOrder(pxPortSpiArr[ucUnitNumber], LL_SPI_LSB_FIRST);
}

/*
 * Sets baudrate prescaler.
 */
static inline void vPort_SPI_setBaudratePrescaler(uint8_t ucUnitNumber, uint16_t usPrescaler)
{
	switch(usPrescaler)
	{
	case 2:
		LL_SPI_SetBaudRatePrescaler(pxPortSpiArr[ucUnitNumber], LL_SPI_BAUDRATEPRESCALER_DIV2);
		break;

	case 4:
		LL_SPI_SetBaudRatePrescaler(pxPortSpiArr[ucUnitNumber], LL_SPI_BAUDRATEPRESCALER_DIV4);
		break;

	case 8:
		LL_SPI_SetBaudRatePrescaler(pxPortSpiArr[ucUnitNumber], LL_SPI_BAUDRATEPRESCALER_DIV8);
		break;

	case 16:
		LL_SPI_SetBaudRatePrescaler(pxPortSpiArr[ucUnitNumber], LL_SPI_BAUDRATEPRESCALER_DIV16);
		break;

	case 32:
		LL_SPI_SetBaudRatePrescaler(pxPortSpiArr[ucUnitNumber], LL_SPI_BAUDRATEPRESCALER_DIV32);
		break;

	case 64:
		LL_SPI_SetBaudRatePrescaler(pxPortSpiArr[ucUnitNumber], LL_SPI_BAUDRATEPRESCALER_DIV64);
		break;

	case 128:
		LL_SPI_SetBaudRatePrescaler(pxPortSpiArr[ucUnitNumber], LL_SPI_BAUDRATEPRESCALER_DIV128);
		break;

	case 256:
		LL_SPI_SetBaudRatePrescaler(pxPortSpiArr[ucUnitNumber], LL_SPI_BAUDRATEPRESCALER_DIV256);
		break;
	default:
		vLib_ASSERT(0, usPORT_ERR_SPI_WRONG_BAUDRATE_PRESCALER);
	}
}

/*
 * Selects the SPI unit as a master.
 */
static inline void vPort_SPI_enableMasterMode(uint8_t ucUnitNumber)
{
	/*	SW slave management enable	*/
	LL_SPI_SetNSSMode(pxPortSpiArr[ucUnitNumber], LL_SPI_NSS_SOFT);

	/*	SW slave management deselect (self deselect) and select master mode	*/
	LL_SPI_SetMode(pxPortSpiArr[ucUnitNumber], LL_SPI_MODE_MASTER);
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
	if (ucMode & 1) // 1st bit is for clock phase
		LL_SPI_SetClockPhase(pxPortSpiArr[ucUnitNumber], LL_SPI_PHASE_2EDGE);
	else
		LL_SPI_SetClockPhase(pxPortSpiArr[ucUnitNumber], LL_SPI_PHASE_1EDGE);

	if (ucMode & 2) // 2nd bit is for clock polarity
		LL_SPI_SetClockPolarity(pxPortSpiArr[ucUnitNumber], LL_SPI_POLARITY_HIGH);
	else
		LL_SPI_SetClockPolarity(pxPortSpiArr[ucUnitNumber], LL_SPI_POLARITY_LOW);
}

/*
 * Enables SPI unit.
 */
static inline void vPort_SPI_enable(uint8_t ucUnitNumber)
{
	LL_SPI_Enable(pxPortSpiArr[ucUnitNumber]);
}


/*
 * Disables SPI unit.
 */
static inline void vPort_SPI_disable(uint8_t ucUnitNumber)
{
	LL_SPI_Disable(pxPortSpiArr[ucUnitNumber]);
}

/*
 * Enables Tx buffer empty interrupt.
 */
static inline void vPORT_SPI_enableTxeInterrupt(uint8_t ucUnitNumber)
{
	LL_SPI_EnableIT_TXE(pxPortSpiArr[ucUnitNumber]);
}

/*
 * Disables Tx buffer empty interrupt.
 */
static inline void vPORT_SPI_disableTxeInterrupt(uint8_t ucUnitNumber)
{
	LL_SPI_DisableIT_TXE(pxPortSpiArr[ucUnitNumber]);
}

/*
 * Enables Rx buffer not empty interrupt.
 */
static inline void vPORT_SPI_enableRxneInterrupt(uint8_t ucUnitNumber)
{
	LL_SPI_EnableIT_RXNE(pxPortSpiArr[ucUnitNumber]);
}

/*
 * Disables Rx buffer not empty interrupt.
 */
static inline void vPORT_SPI_disableRxneInterrupt(uint8_t ucUnitNumber)
{
	LL_SPI_DisableIT_RXNE(pxPortSpiArr[ucUnitNumber]);
}

/*
 * Checks if Tx buffer empty interrupt is enabled.
 */
static inline uint8_t ucPort_SPI_IsTxeInterruptEnabled(uint8_t ucUnitNumber)
{
	return LL_SPI_IsEnabledIT_TXE(pxPortSpiArr[ucUnitNumber]);
}

/*
 * Checks if Rx buffer not empty interrupt is enabled.
 */
static inline uint8_t ucPort_SPI_IsRxneInterruptEnabled(uint8_t ucUnitNumber)
{
	return LL_SPI_IsEnabledIT_RXNE(pxPortSpiArr[ucUnitNumber]);
}

/*
 * Clears RxNE flag.
 */
static inline void vPORT_SPI_clearRxneFlag(uint8_t ucUnitNumber)
{
	(volatile void)LL_SPI_ReceiveData8(pxPortSpiArr[ucUnitNumber]);
}

/*
 * Reads TxE flag.
 */
#define ucPORT_SPI_GET_TXE_FLAG(ucUnitNumber)	\
	(LL_SPI_IsActiveFlag_TXE(pxPortSpiArr[(ucUnitNumber)]))

/*
 * Reads RxNE flag.
 */
#define ucPORT_SPI_GET_RXNE_FLAG(ucUnitNumber)	\
	(LL_SPI_IsActiveFlag_RXNE(pxPortSpiArr[(ucUnitNumber)]))

/*
 * Sets TxE callback.
 */
static inline void vPort_SPI_setTxeCallback(	uint8_t ucUnitNumber,
												void(*pfCallback)(void*),
												void* pvParams	)
{
	ppfPortSpiTxeCallbackArr[ucUnitNumber] = pfCallback;
	ppvPortSpiTxeCallbackParamsArr[ucUnitNumber] = pvParams;
}

/*
 * Sets RxNE callback.
 */
static inline void vPort_SPI_setRxneCallback(	uint8_t ucUnitNumber,
												void(*pfCallback)(void*),
												void* pvParams	)
{
	ppfPortSpiRxneCallbackArr[ucUnitNumber] = pfCallback;
	ppvPortSpiRxneCallbackParamsArr[ucUnitNumber] = pvParams;
}

/*
 * Checks if SPI unit is busy.
 */
static inline uint8_t ucPort_SPI_isBusy(uint8_t ucUnitNumber)
{
	return LL_SPI_IsActiveFlag_BSY(pxPortSpiArr[ucUnitNumber]);
}


/*
 * Writes a byte on the data register. (no waiting).
 */
static inline void vPort_SPI_writeDataNoWait(uint8_t ucUnitNumber, uint8_t ucByte)
{
	LL_SPI_TransmitData8(pxPortSpiArr[ucUnitNumber], ucByte);
}


/*
 * Reads the byte on the data register. (no waiting).
 */
static inline uint8_t ucPort_SPI_readDataNoWait(uint8_t ucUnitNumber)
{
	return LL_SPI_ReceiveData8(pxPortSpiArr[ucUnitNumber]);
}

/*
 * Returns pointer to unit's data register.
 * (Used when DMA is enabled).
 */
static inline void* pvPort_SPI_getDrAddress(uint8_t ucUnitNumber)
{
	return (void*)(&pxPortSpiArr[ucUnitNumber]->DR);
}

/*
 * Connects SPI unit's TxE signal to the given DMA channel.
 *
 * Notes:
 * 		-	If there's no DMA in the used target, this function is ignored.
 *
 * 		-	If the used target has static DMA mapping, the DMA connection passed
 * 			to this function is ignored.
 */
static inline void vPort_SPI_connectTxeToDma(	uint8_t ucSpiUnitNumber,
												uint8_t ucDmaUnitNumber,
												uint8_t ucDmaChannelNumber	)
{
	LL_SPI_EnableDMAReq_TX(pxPortSpiArr[ucSpiUnitNumber]);
}

/*
 * Disconnects SPI unit's TxE signal from DMA.
 *
 * Notes:
 * 		-	If there's no DMA in the used target, this function is ignored.
 */
static inline void vPort_SPI_disconnectTxeFromDma(uint8_t ucSpiUnitNumber)
{
	LL_SPI_DisableDMAReq_TX(pxPortSpiArr[ucSpiUnitNumber]);
}

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

	if (pxHWConf->ucLSBitFirst)
		vPort_SPI_setLSBFirst(ucUnitNumber);
	else
		vPort_SPI_setMSBFirst(ucUnitNumber);

	vPort_SPI_setBaudratePrescaler(ucUnitNumber, pxHWConf->usBaudratePrescaler);

	if (pxHWConf->ucIsMaster)
		vPort_SPI_enableMasterMode(ucUnitNumber);

	vPort_SPI_setComMode(ucUnitNumber, pxHWConf->ucComMode);

	vPort_SPI_setFrameFormat8Bit(ucUnitNumber);

	vPort_SPI_enable(ucUnitNumber);

	vPort_AFIO_mapSpi(ucUnitNumber, pxHWConf->ucAFIOMapNumber);

	vPort_GPIO_initSpiPins(	ucUnitNumber,
							pxHWConf->ucAFIOMapNumber,
							pxHWConf->ucNssEn,
							pxHWConf->ucMISOEn,
							pxHWConf->ucMOSIEn	);
}










#endif /* HAL_OS_PORT_PORT_SPI_H_ */
