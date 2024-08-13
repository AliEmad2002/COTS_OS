/*
 * Port_DMA.h
 *
 *  Created on: Jun 17, 2023
 *      Author: Ali Emad
 */

/*	Target checking	*/
#include "MCAL_Port/Port_Target.h"
#ifdef MCAL_PORT_TARGET_STM32F103C8T6


#ifndef HAL_OS_PORT_PORT_DMA_H_
#define HAL_OS_PORT_PORT_DMA_H_

#include "stm32f1xx.h"
#include "stm32f1xx_ll_dma.h"

/*******************************************************************************
 * Definition of hardware properties:
 ******************************************************************************/
/*
 * If the used target has DMA, make "portDMA_IS_AVAILABLE" for enhanced performance
 * in drivers which do tons of data transfer, like communication drivers, SD-card,
 * graphical display, and other examples.
 *
 * If the used target does not have DMA, make sure that "portDMA_IS_AVAILABLE" is 0.
 */
#define portDMA_IS_AVAILABLE					0


#if portDMA_IS_AVAILABLE

#define portDMA_NUMBER_OF_UNITS					1
#define portDMA_NUMBER_OF_CHANNELS_PER_UNIT		7

/*
 * "portDMA_ENDIANESS" must be defined by one of "portDMA_LITTLE_ENDIAN" and
 * "portDMA_BIG_ENDIAN" based on target's DMA properties.
 */
#define portDMA_LITTLE_ENDIAN	0
#define portDMA_BIG_ENDIAN		1
#define portDMA_ENDIANESS		portDMA_LITTLE_ENDIAN

/*
 * User must edit these arrays in "Port_DMA.c" based on the used  target.
 */
extern DMA_Channel_TypeDef* const pxPort_DmaChArr[portDMA_NUMBER_OF_UNITS][portDMA_NUMBER_OF_CHANNELS_PER_UNIT];
extern DMA_TypeDef* const pxPort_DmaArr[];


/*******************************************************************************
 * Helping structures / Enums:
 ******************************************************************************/
/*
 * Transfer info.
 *
 * Upper layer code must use the following structure to interface DMA. Of course,
 * not all targets have all of the parameters in this structure, hence upper layer
 * writer could ignore whatever parameter his target does not support. (Also, ported
 * DMA template functions must be written properly such they can handle all parameters,
 * ignored and not ignored ones.
 */
typedef struct{
	/*	DMA unit number (see the array "pxPort_DmaChArr")	*/
	uint8_t ucUnitNumber;

	/*	DMA channel number (see the array "pxPort_DmaChArr")	*/
	uint8_t ucChannelNumber;

	/*	Starting addresses	*/
	void* pvMemoryStartingAdderss;
	void* pvPeripheralStartingAdderss;

	/*	Number of data units (bytes) to be transferred	*/
	uint32_t uiN;

	/*	0 ==> HW trigger source, 1==>MEM2MEM (SW triggering)	*/
	uint8_t ucTriggerSource : 1;

//	/*	Transfer data unit sizes. Must be one of "portDMA_DATA_SIZE_xxx" 	*/
//	uint8_t xMemorySize : 2;
//	uint8_t xPeripheralSize : 2;
	/*
	 * For portability, all transfer operations are fixed to a size of 1 byte.
	 */

	/*	Transfer priority level. 0, 1, 2 or 3. Such that 3 is the highest	*/
	uint8_t ucPriLevel : 2;

	/*	0 ==> periph to mem, 1 ==> mem to periph	*/
	uint8_t ucDirection : 1;

	/*
	 * Increment settings.
	 * 0==> No increment, 1==>Increment address.
	 */
	uint8_t ucMemoryIncrement : 1;
	uint8_t ucPeripheralIncrement : 1;
}xPort_DMA_TransInfo_t;

/*******************************************************************************
 * Functions:
 ******************************************************************************/
/*
 * Initializes DMA unit.
 *
 * If the used target does not require unit initialization, this function is left
 * empty. Although, it must be called in upper layer code before using DMA, for
 * portability.
 */
void vPort_DMA_initUnit(uint8_t ucUnitNumber);

/*
 * Initializes DMA channel.
 *
 * If the used target does not require channel initialization, this function is
 * left empty. Although, it must be called in upper layer code before using DMA,
 * for portability.
 */
void vPort_DMA_initChannel(uint8_t ucUnitNumber, uint8_t ucChannelNumber);

/*
 * Initiates new DMA transfer.
 *
 * Notes:
 * 		-	Previously started transfer on the same channel must have been completed
 * 			first before calling this function. An upper layer DMA driver may use
 * 			flag checking or mutexes to achieve that.
 */
void vPort_DMA_startTransfer(xPort_DMA_TransInfo_t*  pxInfo);

/*
 * Reads transfer complete flag of a channel.
 */
#define ucPort_DMA_GET_TC_FLAG(ucUnitNumber, ucChannelNumber)	\
	(READ_BIT(pxPort_DmaArr[(ucUnitNumber)]->ISR, 1 << (4*(ucChannelNumber)+1)))

/*
 * Clears transfer complete flag of a channel.
 */
#define vPort_DMA_CLEAR_TC_FLAG(ucUnitNumber, ucChannelNumber)	\
		WRITE_REG(pxPort_DmaArr[(ucUnitNumber)]->IFCR, 1ul << (1 + 4 * (ucChannelNumber)));

/*
 * Enables transfer complete interrupt of a channel.
 */
#define vPORT_DMA_ENABLE_TRANSFER_COMPLETE_INTERRUPT(ucUnitNumber, ucChannelNumber)	\
	(	LL_DMA_EnableIT_TC(pxPort_DmaArr[(ucUnitNumber)], (ucChannelNumber) + 1)	)

/*
 * Disables transfer complete interrupt of a channel.
 */
#define vPORT_DMA_DISABLE_TRANSFER_COMPLETE_INTERRUPT(ucUnitNumber, ucChannelNumber)	\
	(	LL_DMA_DisableIT_TC(pxPort_DmaArr[(ucUnitNumber)], (ucChannelNumber) + 1)	)

/*
 * Sets callback of transfer complete interrupt of a certain channel.
 */
void vPort_DMA_setTransferCompleteCallback(	uint8_t ucUnitNumber,
											uint8_t ucChannelNumber,
											void(*pfCallback)(void*),
											void* pvParams	);

/*
 * Reads transfer half complete flag of a channel.
 */
#define ucPort_DMA_GET_THC_FLAG(ucUnitNumber, ucChannelNumber)	\
	(READ_BIT(pxPort_DmaArr[(ucUnitNumber)]->ISR, 1 << (4*(ucChannelNumber)+2)))

/*
 * Clears transfer half complete flag of a channel.
 */
#define vPort_DMA_CLEAR_THC_FLAG(ucUnitNumber, ucChannelNumber)	\
		WRITE_REG(pxPort_DmaArr[(ucUnitNumber)]->IFCR, 1ul << (2 + 4 * (ucChannelNumber)));

/*
 * Enables transfer half complete interrupt of a channel.
 */
#define vPORT_DMA_ENABLE_TRANSFER_HALF_COMPLETE_INTERRUPT(ucUnitNumber, ucChannelNumber)	\
	(	LL_DMA_EnableIT_HT(pxPort_DmaArr[(ucUnitNumber)], (ucChannelNumber) + 1)	)

/*
 * Disables transfer half complete interrupt of a channel.
 */
#define vPORT_DMA_DISABLE_TRANSFER_HALF_COMPLETE_INTERRUPT(ucUnitNumber, ucChannelNumber)	\
	(	LL_DMA_DisableIT_HT(pxPort_DmaArr[(ucUnitNumber)], (ucChannelNumber) + 1)	)

/*
 * Sets callback of transfer complete interrupt of a certain channel.
 */
void vPort_DMA_setTransferHalfCompleteCallback(	uint8_t ucUnitNumber,
												uint8_t ucChannelNumber,
												void(*pfCallback)(void*),
												void* pvParams	);




#endif	/*	portDMA_IS_AVAILABLE	*/



















#endif /* HAL_OS_PORT_PORT_DMA_H_ */


#endif /* Target checking */
