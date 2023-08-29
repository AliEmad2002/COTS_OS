/*
 * Port_DMA.c
 *
 *  Created on: Jun 17, 2023
 *      Author: Ali Emad
 */

#include "stm32f1xx.h"
#include "MCAL_Port/Port_DMA.h"

#if portDMA_IS_AVAILABLE

/*
 * This is a multidimensional array of pointers to DMA channels.
 * First dimension specifies DMA unit number, and the second specifies DMA channel
 * number.
 */
DMA_Channel_TypeDef* const pxPort_DmaChArr[portDMA_NUMBER_OF_UNITS][portDMA_NUMBER_OF_CHANNELS_PER_UNIT] = {
	{
		DMA1_Channel1, DMA1_Channel2, DMA1_Channel3, DMA1_Channel4,
		DMA1_Channel5, DMA1_Channel6, DMA1_Channel7
	}
};

/*
 * Array to DMA peripherals.
 */
DMA_TypeDef* const pxPort_DmaArr[] = {DMA1};

/*******************************************************************************
 * Helping functions:
 ******************************************************************************/

/*******************************************************************************
 * Functions:
 ******************************************************************************/
void vPort_DMA_startTransfer(xPort_DMA_TransInfo_t* pxInfo)
{
	uint32_t uiConf;

	DMA_TypeDef* pxUnitHandle = pxPort_DmaArr[pxInfo->ucUnitNumber];
	uint32_t uiChannelNumber = pxInfo->ucChannelNumber + 1;

	/*	Disable channel	*/
	LL_DMA_DisableChannel(pxUnitHandle, uiChannelNumber);

	/*	Set transfer direction SW trigger enable	*/
	if (pxInfo->ucDirection == 0)
		uiConf = LL_DMA_DIRECTION_PERIPH_TO_MEMORY;
	else
		uiConf = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;

	if (pxInfo->ucTriggerSource == 1)
		uiConf |= LL_DMA_DIRECTION_MEMORY_TO_MEMORY;

	LL_DMA_SetDataTransferDirection(pxUnitHandle, uiChannelNumber, uiConf);

	/*	Write peripheral increment setting	*/
	if (pxInfo->ucPeripheralIncrement == 0)
		uiConf = LL_DMA_PERIPH_NOINCREMENT;
	else
		uiConf = LL_DMA_PERIPH_INCREMENT;

	LL_DMA_SetPeriphIncMode(pxUnitHandle, uiChannelNumber, uiConf);

	/*	Write memory increment setting	*/
	if (pxInfo->ucMemoryIncrement == 0)
		uiConf = LL_DMA_MEMORY_NOINCREMENT;
	else
		uiConf = LL_DMA_MEMORY_INCREMENT;

	LL_DMA_SetMemoryIncMode(pxUnitHandle, uiChannelNumber, uiConf);

	/*	Write peripheral data unit size to 1 byte	*/
	LL_DMA_SetPeriphSize(pxUnitHandle, uiChannelNumber, LL_DMA_PDATAALIGN_BYTE);

	/*	Write memory data unit size	*/
	LL_DMA_SetMemorySize(pxUnitHandle, uiChannelNumber, LL_DMA_PDATAALIGN_BYTE);

	/*	Write priority setting	*/
	uiConf = pxInfo->ucPriLevel << DMA_CCR_PL_Pos;
	LL_DMA_SetChannelPriorityLevel(pxUnitHandle, uiChannelNumber, uiConf);

	/*	Write number of data unit to be transferred	*/
	LL_DMA_SetDataLength(pxUnitHandle, uiChannelNumber, pxInfo->uiN);

	/*	Write starting addresses	*/
	uiConf = (uint32_t)pxInfo->pvPeripheralStartingAdderss;
	LL_DMA_SetPeriphAddress(pxUnitHandle, uiChannelNumber, uiConf);

	uiConf = (uint32_t)pxInfo->pvMemoryStartingAdderss;
	LL_DMA_SetMemoryAddress(pxUnitHandle, uiChannelNumber, uiConf);

	/*	Enable channel	*/
	LL_DMA_EnableChannel(pxUnitHandle, uiChannelNumber);
}

/*******************************************************************************
 * ISRs:
 ******************************************************************************/
volatile void(*ppfPortDmaCallbackArr[portDMA_NUMBER_OF_UNITS][portDMA_NUMBER_OF_CHANNELS_PER_UNIT]) (void*);
void* ppvPortDmaCallbackParamsArr[portDMA_NUMBER_OF_UNITS][portDMA_NUMBER_OF_CHANNELS_PER_UNIT];

/*******************************************************************************
 * ISRs:
 *
 * Notes
 * 		-	Define them as shown, target dependent.
 * 		-	Add clearing pending flag to the end of the ISR
 ******************************************************************************/
void DMA1_Channel1_IRQHandler(void)
{
	if (ucPort_DMA_GET_TC_FLAG(0, 0))
	{
		ppfPortDmaCallbackArr[0][0](ppvPortDmaCallbackParamsArr[0][0]);
		vPort_DMA_CLEAR_TC_FLAG(0, 0);
	}
}
void DMA1_Channel2_IRQHandler(void)
{
	if (ucPort_DMA_GET_TC_FLAG(0, 1))
	{
		ppfPortDmaCallbackArr[0][1](ppvPortDmaCallbackParamsArr[0][1]);
		vPort_DMA_CLEAR_TC_FLAG(0, 1);
	}
}
void DMA1_Channel3_IRQHandler(void)
{
	if (ucPort_DMA_GET_TC_FLAG(0, 2))
	{
		ppfPortDmaCallbackArr[0][2](ppvPortDmaCallbackParamsArr[0][2]);
		vPort_DMA_CLEAR_TC_FLAG(0, 2);
	}
}
void DMA1_Channel4_IRQHandler(void)
{
if (ucPort_DMA_GET_TC_FLAG(0, 3))
	{
		ppfPortDmaCallbackArr[0][3](ppvPortDmaCallbackParamsArr[0][3]);
		vPort_DMA_CLEAR_TC_FLAG(0, 3);
	}
}
void DMA1_Channel5_IRQHandler(void)
{
	if (ucPort_DMA_GET_TC_FLAG(0, 4))
	{
		ppfPortDmaCallbackArr[0][4](ppvPortDmaCallbackParamsArr[0][4]);
		vPort_DMA_CLEAR_TC_FLAG(0, 4);
	}
}
void DMA1_Channel6_IRQHandler(void)
{
	if (ucPort_DMA_GET_TC_FLAG(0, 5))
	{
		ppfPortDmaCallbackArr[0][5](ppvPortDmaCallbackParamsArr[0][5]);
		vPort_DMA_CLEAR_TC_FLAG(0, 5);
	}
}
void DMA1_Channel7_IRQHandler(void)
{
	if (ucPort_DMA_GET_TC_FLAG(0, 6))
	{
		ppfPortDmaCallbackArr[0][6](ppvPortDmaCallbackParamsArr[0][6]);
		vPort_DMA_CLEAR_TC_FLAG(0, 6);
	}
}


#endif		/*		portDMA_IS_AVAILABLE		*/
