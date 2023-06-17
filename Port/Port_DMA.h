/*
 * Port_DMA.h
 *
 *  Created on: Jun 17, 2023
 *      Author: Ali Emad
 */

#ifndef HAL_OS_PORT_PORT_DMA_H_
#define HAL_OS_PORT_PORT_DMA_H_

#include "stm32f1xx.h"
#include "Bit_Math.h"

/*******************************************************************************
 * Definition of hardware properties:
 ******************************************************************************/
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
extern DMA_Channel_TypeDef* const pxPort_DmaChArr[1][7];
extern DMA_TypeDef* const pxPort_DmaArr[];

/*******************************************************************************
 * Helping structures:
 ******************************************************************************/
/*
 * This structure would work with most of STM32 DMA's
 * A major change would be needed in case of an odd MCU used. Affecting upper layers code!
 */
typedef struct{
	/*	DMA unit number (see the array "pxPort_DmaChArr")	*/
	uint8_t ucUnitNumber;

	/*	DMA channel number (see the array "pxPort_DmaChArr")	*/
	uint8_t ucChannelNumber;

	/*	Starting addresses	*/
	void* pvMemoryStartingAdderss;
	void* pvPeripheralStartingAdderss;

	/*	Number of data units to be transferred	*/
	uint32_t uiN;

	/*	0 ==> HW trigger source, 1==>MEM2MEM (SW triggering)	*/
	uint8_t ucTriggerSource : 1;

	/*	Transfer data unit sizes. Must be one of "portDMA_DATA_SIZE_xxx" 	*/
	uint8_t xMemorySize : 2;
	uint8_t xPeripheralSize : 2;

	/*	Transfer priority level. 0, 1, 2 or 3. Such that 3 is the highest	*/
	uint8_t ucPriLevel : 2;

	/*	0 ==> periph to mem, 1 ==> mem to periph	*/
	uint8_t ucDirection : 1;

	/*	Increment settings	*/
	uint8_t ucMemoryIncrement : 1;
	uint8_t ucPeripheralIncrement : 1;
}xPort_DMA_TransInfo_t;

/*******************************************************************************
 * Functions:
 ******************************************************************************/
/*
 * Initiates new DMA transfer.
 *
 * Notes:
 * 		-	Previously started transfer on the same channel must have been completed
 * 			first before calling this function.
 */
void vPort_DMA_startTransfer(xPort_DMA_TransInfo_t*  pxInfo);

/*
 * Reads transfer complete flag.
 */
#define ucPort_DMA_GET_TC_FLAG(ucUnitNumber, ucChannelNumber)	\
(ucGET_BIT(pxPort_DmaArr[(ucUnitNumber)]->ISR, 4 * (ucChannelNumber) + 1))

/*
 * Enables transfer complete interrupt of a channel.
 *
 * Notes:
 * 		-	If the target MCU does not have a unique enable selection for the TC interrupt,
 * 			the following wrapper must then enable DMA's global interrupt. As anyways,
 * 			upper layer drivers use flag checking macros to know whether TC is the
 * 			cause of interrupt or not.
 */
static inline void vPort_DMA_enableTransferCompleteInterrupt(uint8_t ucUnitNumber, uint8_t ucChannelNumber)
{
	vSET_BIT(pxPort_DmaChArr[pxInfo->ucUnitNumber][pxInfo->ucChannelNumber]->CCR, 1);
}

/*
 * Disables transfer complete interrupt of a channel.
 */
static inline void vPort_DMA_disableTransferCompleteInterrupt(uint8_t ucUnitNumber, uint8_t ucChannelNumber)
{
	vCLR_BIT(pxPort_DmaChArr[pxInfo->ucUnitNumber][pxInfo->ucChannelNumber]->CCR, 1);
}























#endif /* HAL_OS_PORT_PORT_DMA_H_ */
