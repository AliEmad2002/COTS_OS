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
#include "MCAL/Inc/SPI/SPI_interface.h"

/*
 * Enables unidirectional mode.
 * i.e: makes communication via two lines (full-duplex).
 */
static inline void vHOS_SPI_setFullDuplex(uint8_t ucUnitNumber)
{
	SPI_SET_FULL_DUPLEX(ucUnitNumber);
}

/*
 * Sets frame-format to 8-bits.
 */
static inline void vHOS_SPI_setFrameFormat8Bit(uint8_t ucUnitNumber)
{
	SPI_SET_FRAME_FORMAT_8_BIT_NO_WAIT(ucUnitNumber);
}

/*
 * Sets frame direction to MSB_First.
 */
static inline void vHOS_SPI_setMSBFirst(uint8_t ucUnitNumber)
{
	SPI_SET_MSB_FIRST(ucUnitNumber);
}


/*
 * Sets frame direction to LSB_First.
 */
static inline void vHOS_SPI_setLSBFirst(uint8_t ucUnitNumber)
{
	SPI_SET_LSB_FIRST(ucUnitNumber);
}

/*
 * Sets baudrate prescaler.
 */
static inline void vHOS_SPI_setBaudratePrescaler(uint8_t ucUnitNumber, uint16_t usPrescaler)
{
	SPI_voidSetBaudRatePrescalerNumericNoWait(ucUnitNumber, usPrescaler);
}

/*
 * Selects the SPI unit as a master.
 */
static inline void vHOS_SPI_enableMasterMode(uint8_t ucUnitNumber)
{
	SPI_ENABLE_MASTER_MODE(ucUnitNumber);
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
static inline void vHOS_SPI_setComMode(uint8_t ucUnitNumber, uint8_t ucMode)
{
	SPI_SET_COM_MODE(ucUnitNumber, ucMode);
}

/*
 * Initializes GPIO pins which correspond to the SPI unit.
 *
 * Notes:
 * 		-	user may omit (ignore) the last 4 arguments if they are of no use
 * 		in their port target.
 */
static inline void vHOS_SPI_initGPIO(	uint8_t ucUnitNumber,
										uint8_t ucMap,		/*	AFIO map number	*/
										uint8_t icInitNSS,	/*	set to 1 if NSS pin is needed	*/
										uint8_t icInitMISO,	/*	set to 1 if MISO pin is needed	*/
										uint8_t icInitMOSI	/*	set to 1 if MOSI pin is needed	*/
										)
{
	SPI_voidInitPins(ucUnitNumber, ucMap, icInitNSS, icInitMISO, icInitMOSI);
}

/*
 * Enables SPI unit.
 */
static inline void vHOS_SPI_enable(uint8_t ucUnitNumber)
{
	SPI_ENABLE_PERIPHERAL(ucUnitNumber);
}


/*
 * Disables SPI unit.
 */
static inline void vHOS_SPI_disable(uint8_t ucUnitNumber)
{
	SPI_DISABLE_PERIPHERAL(ucUnitNumber);
}

/*
 * Enables transfer complete interrupt.
 */
static inline void vHOS_SPI_enableTransferCompleteInterrupt(uint8_t ucUnitNumber)
{
	SPI_voidEnableInterrupt(ucUnitNumber, SPI_Interrupt_TxEmpty);
	vHOS_Interrupt_enableIRQ(35 + ucUnitNumber);
}

/*
 * Checks if SPI unit is busy.
 */
#define ucHOS_SPI_IS_BUSY(ucUnitNumber)	(SPI_IS_BUSY(ucUnitNumber))

/*
 * Writes a byte on the data register. (no waiting).
 */
#define vHOS_SPI_WRT_DR_NO_WAIT(ucUnitNumber, ucByte)	\
		SPI_WRT_DR_NO_WAIT(ucUnitNumber, ucByte)


/*
 * Reads the byte on the data register. (no waiting).
 */
#define vHOS_SPI_GET_DR_NO_WAIT(ucUnitNumber)	\
		SPI_GET_DR_NO_WAIT(ucUnitNumber)















#endif /* HAL_OS_PORT_PORT_SPI_H_ */
