/*
 * Port_I2C.h
 *
 *  Created on: Jul 9, 2023
 *      Author: Ali Emad
 */

#ifndef COTS_OS_INC_MCAL_PORT_PORT_I2C_H_
#define COTS_OS_INC_MCAL_PORT_PORT_I2C_H_

#include "stm32f103xb.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_ll_i2c.h"
#include "MCAL_Port/Port_Clock.h"
#include "MCAL_Port/Port_AFIO.h"
#include "MCAL_Port/Port_GPIO.h"
#include "LIB/Assert.h"

extern I2C_TypeDef* const pxPortI2cArr[];

/*******************************************************************************
 * Configurations:
 ******************************************************************************/
/*
 * Configures whether enabling or disabling ACK bit would take effect on the
 * currently being received byte (value = 0), or the one next to it (value = 1).
 *
 * Some devices may force to one option. just configure this macro to it.
 */
#define ucPORT_I2C_ACK_POS			0

/*******************************************************************************
 * Structures:
 ******************************************************************************/
typedef struct{
	uint8_t ucAFIOMapNumber;
	uint8_t ucEnableGeneralCall : 1;
	uint8_t ucEnableClockStretching : 1;
	uint8_t ucClockMode: 1; // 0==>SM, 1==>FM
	uint8_t ucIsAddress7Bit: 1;
	uint16_t usSelfAddress : 10;
	uint32_t uiSclFrequencyHz;
	uint32_t uiMaxRisingTimeNs;
}xPort_I2C_HW_Conf_t;

/*******************************************************************************
 * API functions:
 ******************************************************************************/
/*	Configures HW on startup	*/
void vPort_I2C_initHardware(uint8_t ucUnitNumber, xPort_I2C_HW_Conf_t* pxConf);

/*	Enable I2C unit	*/
static inline void vPort_I2C_enable(uint8_t ucUnitNumber)
{
	/*	Enable peripheral	*/
	LL_I2C_Enable(pxPortI2cArr[ucUnitNumber]);

	/*	select I2C mode (not SMBus mode)	*/
	LL_I2C_SetMode(pxPortI2cArr[ucUnitNumber], LL_I2C_MODE_I2C);
}

/*	Disable I2C unit	*/
static inline void vPort_I2C_disable(uint8_t ucUnitNumber)
{
	LL_I2C_Disable(pxPortI2cArr[ucUnitNumber]);
}

/*	Enable general call	*/
static inline void vPort_I2C_enableGeneralCall(uint8_t ucUnitNumber)
{
	LL_I2C_EnableGeneralCall(pxPortI2cArr[ucUnitNumber]);
}

/*	Disable general call	*/
static inline void vPort_I2C_disableGeneralCall(uint8_t ucUnitNumber)
{
	LL_I2C_DisableGeneralCall(pxPortI2cArr[ucUnitNumber]);
}

/*	Enable clock stretching	*/
static inline void vPort_I2C_enableClockStretching(uint8_t ucUnitNumber)
{
	LL_I2C_EnableClockStretching(pxPortI2cArr[ucUnitNumber]);
}

/*	Disable clock stretching	*/
static inline void vPort_I2C_disableClockStretching(uint8_t ucUnitNumber)
{
	LL_I2C_DisableClockStretching(pxPortI2cArr[ucUnitNumber]);
}

/*	Enter master mode and generate a start condition	*/
static inline void vPort_I2C_generateStart(uint8_t ucUnitNumber)
{
	LL_I2C_GenerateStartCondition(pxPortI2cArr[ucUnitNumber]);
}

/*
 * Generate a stop condition and exit master mode.
 * (Notice: In master receiver mode, this function is called right after receiving
 * the before-last byte)
 */
static inline void vPort_I2C_generateStop(uint8_t ucUnitNumber)
{
	LL_I2C_GenerateStopCondition(pxPortI2cArr[ucUnitNumber]);
}

/*	Release the I2C bus when in slave transmitter mode	*/
static inline void vPort_I2C_slaveReleaseBus(uint8_t ucUnitNumber)
{
	LL_I2C_GenerateStopCondition(pxPortI2cArr[ucUnitNumber]);
}

/*	Enable ACK (an acknowledge would be sent after receiving a byte)	*/
static inline void vPort_I2C_enableAck(uint8_t ucUnitNumber)
{
	LL_I2C_AcknowledgeNextData(pxPortI2cArr[ucUnitNumber], LL_I2C_ACK);
}

/*	Disable ACK (a non-acknowledge would be sent after receiving a byte)	*/
static inline void vPort_I2C_disableAck(uint8_t ucUnitNumber)
{
	LL_I2C_AcknowledgeNextData(pxPortI2cArr[ucUnitNumber], LL_I2C_NACK);
}

/*
 * Select ACK setting effect position (refer to description of "ucPORT_I2C_ACK_POS"
 * above.
 */
static inline void vPort_I2C_setAckPos(uint8_t ucUnitNumber, uint8_t ucPos)
{
	if (ucPos)
		LL_I2C_EnableBitPOS(pxPortI2cArr[ucUnitNumber]);
	else
		LL_I2C_DisableBitPOS(pxPortI2cArr[ucUnitNumber]);
}

/*	Initialize vendor specific configurations	*/
static inline void vPort_I2C_init(uint8_t ucUnitNumber)
{
	/*	Set peripheral frequency to that of APB bus	*/
	LL_I2C_SetPeriphClock(
		pxPortI2cArr[ucUnitNumber],
		uiPORT_CLOCK_MAIN_HZ / 1 / 2);
		//uiPORT_CLOCK_MAIN_HZ / uiPORT_CLOCK_AHB_DIV / uiPORT_CLOCK_APB1_DIV	);
}

/*
 * Enable interrupt
 * (for portability, it is assumed that I2C unit has one vector only. Flag checking
 * is done inside the handler).
 */
static inline void vPort_I2C_enableInterrupt(uint8_t ucUnitNumber)
{
	LL_I2C_EnableIT_TX(pxPortI2cArr[ucUnitNumber]);
	LL_I2C_EnableIT_ERR(pxPortI2cArr[ucUnitNumber]);
}

/*	Disable interrupt	*/
static inline void vPort_I2C_disableInterrupt(uint8_t ucUnitNumber)
{
	LL_I2C_DisableIT_TX(pxPortI2cArr[ucUnitNumber]);
	LL_I2C_DisableIT_ERR(pxPortI2cArr[ucUnitNumber]);
}

/*	Write own slave address	*/
static inline void vPort_I2C_writeOwnSlaveAddress(	uint8_t ucUnitNumber,
													uint16_t usAddress,
													uint8_t ucIs7Bit	)
{
	LL_I2C_SetOwnAddress1(
		pxPortI2cArr[ucUnitNumber],
		usAddress,
		ucIs7Bit ? LL_I2C_OWNADDRESS1_7BIT : LL_I2C_OWNADDRESS1_10BIT	);
}

/*	Read DR immediately	*/
static inline uint8_t ucPort_I2C_readDrImm(uint8_t ucUnitNumber)
{
	return LL_I2C_ReceiveData8(pxPortI2cArr[ucUnitNumber]);
}

/*	Write DR immediately	*/
static inline void vPort_I2C_writeDrImm(uint8_t ucUnitNumber, uint8_t ucDR)
{
	return LL_I2C_TransmitData8(pxPortI2cArr[ucUnitNumber], ucDR);
}

/*	Read Over-run flag	*/
static inline uint8_t ucPort_I2C_readOvrFlag(uint8_t ucUnitNumber)
{
	return LL_I2C_IsActiveFlag_OVR(pxPortI2cArr[ucUnitNumber]);
}

/*	Clear Over-run flag	*/
static inline void vPort_I2C_clearOvrFlag(uint8_t ucUnitNumber)
{
	LL_I2C_ClearFlag_OVR(pxPortI2cArr[ucUnitNumber]);
}

/*	Read ACK error flag	*/
static inline uint8_t ucPort_I2C_readAckErrFlag(uint8_t ucUnitNumber)
{
	return LL_I2C_IsActiveFlag_AF(pxPortI2cArr[ucUnitNumber]);
}

/*	Clear ACK error flag	*/
static inline void vPort_I2C_clearAckErrFlag(uint8_t ucUnitNumber)
{
	LL_I2C_ClearFlag_AF(pxPortI2cArr[ucUnitNumber]);
}

/*	Read arbitration lost flag	*/
static inline uint8_t ucPort_I2C_readArbitrationLostFlag(uint8_t ucUnitNumber)
{
	return LL_I2C_IsActiveFlag_ARLO(pxPortI2cArr[ucUnitNumber]);
}

/*	Clear arbitration lost flag	*/
static inline void vPort_I2C_clearArbitrationLostFlag(uint8_t ucUnitNumber)
{
	LL_I2C_ClearFlag_ARLO(pxPortI2cArr[ucUnitNumber]);
}

/*	Read bus error flag	*/
static inline uint8_t ucPort_I2C_readBusErrorFlag(uint8_t ucUnitNumber)
{
	return LL_I2C_IsActiveFlag_BERR(pxPortI2cArr[ucUnitNumber]);
}

/*	Clear bus error flag	*/
static inline void vPort_I2C_clearBusErrorFlag(uint8_t ucUnitNumber)
{
	LL_I2C_ClearFlag_BERR(pxPortI2cArr[ucUnitNumber]);
}

/*	Read DR empty (in transmitter mode) flag	*/
static inline uint8_t ucPort_I2C_readTxEmptyFlag(uint8_t ucUnitNumber)
{
	return LL_I2C_IsActiveFlag_TXE(pxPortI2cArr[ucUnitNumber]);
}

/*	Clear DR empty (in transmitter mode) flag	*/
static inline void vPort_I2C_clearTxEmptyFlag(uint8_t ucUnitNumber)
{
	/*	Not available in STM32F1x	*/
}

/*	Read DR not empty (in receiver mode) flag	*/
static inline uint8_t ucPort_I2C_readRxNotEmptyFlag(uint8_t ucUnitNumber)
{
	return LL_I2C_IsActiveFlag_RXNE(pxPortI2cArr[ucUnitNumber]);
}

/*	Clear DR not empty (in receiver mode) flag	*/
static inline void vPort_I2C_clearRxNotEmptyFlag(uint8_t ucUnitNumber)
{
	/*	Not available in STM32F1x	*/
}

/*	Read stop condition detection flag	*/
static inline uint8_t ucPort_I2C_readStopCondFlag(uint8_t ucUnitNumber)
{
	return LL_I2C_IsActiveFlag_STOP(pxPortI2cArr[ucUnitNumber]);
}

/*	Clear stop condition detection flag	*/
static inline void vPort_I2C_clearStopCondFlag(uint8_t ucUnitNumber)
{
	LL_I2C_ClearFlag_STOP(pxPortI2cArr[ucUnitNumber]);
}

/*	Read 10-bit header transfer completion (master mode) flag	*/
static inline uint8_t ucPort_I2C_read10BitHeaderTxComplete(uint8_t ucUnitNumber)
{
	return LL_I2C_IsActiveFlag_ADD10(pxPortI2cArr[ucUnitNumber]);
}

/*	Clear 10-bit header transfer completion (master mode) flag	*/
static inline void vPort_I2C_clear10BitHeaderTxComplete(uint8_t ucUnitNumber)
{
	/*	Not available in STM32F1x	*/
}

/*	Read byte transfer finished flag	*/
static inline uint8_t ucPort_I2C_readByteTransferFinishedFlag(uint8_t ucUnitNumber)
{
	return LL_I2C_IsActiveFlag_BTF(pxPortI2cArr[ucUnitNumber]);
}

/*	Clear byte transfer finished flag	*/
static inline void vPort_I2C_clearByteTransferFinishedFlag(uint8_t ucUnitNumber)
{
	/*	Not available in STM32F1x	*/
}

/*	Read address transmission completed (master mode) flag	*/
static inline uint8_t ucPort_I2C_readAddressTxCompleteFlag(uint8_t ucUnitNumber)
{
	return LL_I2C_IsActiveFlag_ADDR(pxPortI2cArr[ucUnitNumber]);
}

/*	Clear address transmission completed (master mode) flag	*/
static inline void vPort_I2C_clearAddressTxCompleteFlag(uint8_t ucUnitNumber)
{
	LL_I2C_ClearFlag_ADDR(pxPortI2cArr[ucUnitNumber]);
}

/*	Read self address received (slave mode) flag	*/
static inline uint8_t ucPort_I2C_readSelfAddressReceivedFlag(uint8_t ucUnitNumber)
{
	return LL_I2C_IsActiveFlag_ADDR(pxPortI2cArr[ucUnitNumber]);
}

/*	Clear self address received (slave mode) flag	*/
static inline void vPort_I2C_clearSelfAddressReceivedFlag(uint8_t ucUnitNumber)
{
	LL_I2C_ClearFlag_ADDR(pxPortI2cArr[ucUnitNumber]);
}

/*	Read start condition transmission completed (master mode) flag	*/
static inline uint8_t ucPort_I2C_readStartConditionTxCompleteFlag(uint8_t ucUnitNumber)
{
	return LL_I2C_IsActiveFlag_SB(pxPortI2cArr[ucUnitNumber]);
}

/*	Clear start condition transmission completed (master mode) flag	*/
static inline void vPort_I2C_clearStartConditionTxCompleteFlag(uint8_t ucUnitNumber)
{
	/*	Not available in STM32F1x	*/
}

/*	Read general call detection flag	*/
static inline uint8_t ucPort_I2C_readGeneralCallFlag(uint8_t ucUnitNumber)
{
	return LL_I2C_IsActiveFlag_GENCALL(pxPortI2cArr[ucUnitNumber]);
}

/*	Clear general call detection flag	*/
static inline void vPort_I2C_clearGeneralCallFlag(uint8_t ucUnitNumber)
{
	/*	Not available in STM32F1x	*/
}

/*
 * Read R/W bit (slave mode) flag
 * (1==> master read, 0==> master write)
 */
static inline uint8_t ucPort_I2C_readRWBitFlag(uint8_t ucUnitNumber)
{
	return LL_I2C_GetTransferDirection(pxPortI2cArr[ucUnitNumber]) ? 1 : 0;
}

/*
 * Clear R/W bit (slave mode) flag
 * (1==> master read, 0==> master write)
 */
static inline void vPort_I2C_clearRWBitFlag(uint8_t ucUnitNumber)
{
	/*	Not available in STM32F1x	*/
}

/*	Read bus busy flag	*/
static inline uint8_t ucPort_I2C_readBusBusyFlag(uint8_t ucUnitNumber)
{
	return LL_I2C_IsActiveFlag_BUSY(pxPortI2cArr[ucUnitNumber]);
}

/*	Clear bus busy flag	*/
static inline void vPort_I2C_clearBusBusyFlag(uint8_t ucUnitNumber)
{
	/*	Not available in STM32F1x	*/
}

/*
 * Set clock mode and frequency.
 * ucMode: 0==>SM, 1==>FM.
 */
static inline void vPort_I2C_setClockModeAndFreq(	uint8_t ucUnitNumber,
													uint8_t ucMode,
													uint32_t uiFreq	)
{
	LL_I2C_SetClockSpeedMode(
		pxPortI2cArr[ucUnitNumber],
		ucMode ? LL_I2C_CLOCK_SPEED_FAST_MODE : LL_I2C_CLOCK_SPEED_STANDARD_MODE);

	uint16_t usCCR;

	if (ucMode == 0) //SM
		usCCR = (uiPORT_CLOCK_MAIN_HZ / 1 / 2) / (2 * uiFreq);

	else
		usCCR = (uiPORT_CLOCK_MAIN_HZ / 1 / 2) / (3 * uiFreq);

	vLib_ASSERT (usCCR < 4096, 0);	// check if frequency is achievable.

	LL_I2C_SetClockPeriod(pxPortI2cArr[ucUnitNumber], usCCR);
}

/*
 * Set maximum rising edge time (a bus property).
 * uiTRise: rising edge time in nanoseconds.
 */
static inline void vPort_I2C_setMaxRisingTime(uint8_t ucUnitNumber, uint32_t uiTRise)
{
	LL_I2C_SetRiseTime(
		pxPortI2cArr[ucUnitNumber],
		((uiPORT_CLOCK_MAIN_HZ / 1 / 2) * (uint64_t)uiTRise) / 1000000000);
}

/*******************************************************************************
 * Interrupt handlers
 ******************************************************************************/
#define port_I2C_HANDLER_0		I2C1_EV_IRQHandler
#define port_I2C_HANDLER_1		I2C2_EV_IRQHandler
















#endif /* COTS_OS_INC_MCAL_PORT_PORT_I2C_H_ */
