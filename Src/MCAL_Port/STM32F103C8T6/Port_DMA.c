/*
 * Port_DMA.c
 *
 *  Created on: Jun 17, 2023
 *      Author: Ali Emad
 */

/*	Target checking	*/
#include "MCAL_Port/Port_Target.h"
#ifdef MCAL_PORT_TARGET_STM32F103C8T6


#include "stm32f1xx.h"
#include "MCAL_Port/Port_DMA.h"
#include "MCAL_Port/Port_Interrupt.h"

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
void vPort_DMA_initUnit(uint8_t ucUnitNumber)
{

}

void vPort_DMA_initChannel(uint8_t ucUnitNumber, uint8_t ucChannelNumber)
{

}

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
 *
 * Notes
 * 		-	Define them as shown, target dependent.
 * 		-	Add clearing pending flag to the end of the ISR
 ******************************************************************************/
#ifdef ucPORT_INTERRUPT_IRQ_DEF_DMA

volatile void(*ppfPortDmaTCCallbackArr[portDMA_NUMBER_OF_UNITS][portDMA_NUMBER_OF_CHANNELS_PER_UNIT]) (void*);
void* ppvPortDmaTCCallbackParamsArr[portDMA_NUMBER_OF_UNITS][portDMA_NUMBER_OF_CHANNELS_PER_UNIT];

volatile void(*ppfPortDmaTHCCallbackArr[portDMA_NUMBER_OF_UNITS][portDMA_NUMBER_OF_CHANNELS_PER_UNIT]) (void*);
void* ppvPortDmaTHCCallbackParamsArr[portDMA_NUMBER_OF_UNITS][portDMA_NUMBER_OF_CHANNELS_PER_UNIT];

void vPort_DMA_setTransferCompleteCallback(	uint8_t ucUnitNumber,
															uint8_t ucChannelNumber,
															void(*pfCallback)(void*),
															void* pvParams	)
{
	ppfPortDmaTCCallbackArr[ucUnitNumber][ucChannelNumber] = pfCallback;
	ppvPortDmaTCCallbackParamsArr[ucUnitNumber][ucChannelNumber] = pvParams;
}

void vPort_DMA_setTransferHalfCompleteCallback(	uint8_t ucUnitNumber,
												uint8_t ucChannelNumber,
												void(*pfCallback)(void*),
												void* pvParams	)
{
	ppfPortDmaTHCCallbackArr[ucUnitNumber][ucChannelNumber] = pfCallback;
	ppvPortDmaTHCCallbackParamsArr[ucUnitNumber][ucChannelNumber] = pvParams;
}

void DMA1_Channel1_IRQHandler(void)
{
	if (ucPort_DMA_GET_TC_FLAG(0, 0))
	{
		ppfPortDmaTCCallbackArr[0][0](ppvPortDmaTCCallbackParamsArr[0][0]);
		vPort_DMA_CLEAR_TC_FLAG(0, 0);
	}
	else if (ucPort_DMA_GET_THC_FLAG(0, 0))
	{
		ppfPortDmaTHCCallbackArr[0][0](ppvPortDmaTCCallbackParamsArr[0][0]);
		vPort_DMA_CLEAR_THC_FLAG(0, 0);
	}
}

void DMA1_Channel2_IRQHandler(void)
{
	if (ucPort_DMA_GET_TC_FLAG(0, 1))
	{
		ppfPortDmaTCCallbackArr[0][1](ppvPortDmaTCCallbackParamsArr[0][1]);
		vPort_DMA_CLEAR_TC_FLAG(0, 1);
	}
	else if (ucPort_DMA_GET_THC_FLAG(0, 1))
	{
		ppfPortDmaTHCCallbackArr[0][1](ppvPortDmaTCCallbackParamsArr[0][1]);
		vPort_DMA_CLEAR_THC_FLAG(0, 1);
	}
}
void DMA1_Channel3_IRQHandler(void)
{
	if (ucPort_DMA_GET_TC_FLAG(0, 2))
	{
		ppfPortDmaTCCallbackArr[0][2](ppvPortDmaTCCallbackParamsArr[0][2]);
		vPort_DMA_CLEAR_TC_FLAG(0, 2);
	}
	else if (ucPort_DMA_GET_THC_FLAG(0, 2))
	{
		ppfPortDmaTHCCallbackArr[0][2](ppvPortDmaTCCallbackParamsArr[0][2]);
		vPort_DMA_CLEAR_THC_FLAG(0, 2);
	}
}
void DMA1_Channel4_IRQHandler(void)
{
	if (ucPort_DMA_GET_TC_FLAG(0, 3))
	{
		ppfPortDmaTCCallbackArr[0][3](ppvPortDmaTCCallbackParamsArr[0][3]);
		vPort_DMA_CLEAR_TC_FLAG(0, 3);
	}
	else if (ucPort_DMA_GET_THC_FLAG(0, 3))
	{
		ppfPortDmaTHCCallbackArr[0][3](ppvPortDmaTCCallbackParamsArr[0][3]);
		vPort_DMA_CLEAR_THC_FLAG(0, 3);
	}
}
void DMA1_Channel5_IRQHandler(void)
{
	if (ucPort_DMA_GET_TC_FLAG(0, 4))
	{
		ppfPortDmaTCCallbackArr[0][4](ppvPortDmaTCCallbackParamsArr[0][4]);
		vPort_DMA_CLEAR_TC_FLAG(0, 4);
	}
	else if (ucPort_DMA_GET_THC_FLAG(0, 4))
	{
		ppfPortDmaTHCCallbackArr[0][4](ppvPortDmaTCCallbackParamsArr[0][4]);
		vPort_DMA_CLEAR_THC_FLAG(0, 4);
	}
}
void DMA1_Channel6_IRQHandler(void)
{
	if (ucPort_DMA_GET_TC_FLAG(0, 5))
	{
		ppfPortDmaTCCallbackArr[0][5](ppvPortDmaTCCallbackParamsArr[0][5]);
		vPort_DMA_CLEAR_TC_FLAG(0, 5);
	}
	else if (ucPort_DMA_GET_THC_FLAG(0, 5))
	{
		ppfPortDmaTHCCallbackArr[0][5](ppvPortDmaTCCallbackParamsArr[0][5]);
		vPort_DMA_CLEAR_THC_FLAG(0, 5);
	}
}
void DMA1_Channel7_IRQHandler(void)
{
	if (ucPort_DMA_GET_TC_FLAG(0, 6))
	{
		ppfPortDmaTCCallbackArr[0][6](ppvPortDmaTCCallbackParamsArr[0][6]);
		vPort_DMA_CLEAR_TC_FLAG(0, 6);
	}
	else if (ucPort_DMA_GET_THC_FLAG(0, 6))
	{
		ppfPortDmaTHCCallbackArr[0][6](ppvPortDmaTCCallbackParamsArr[0][6]);
		vPort_DMA_CLEAR_THC_FLAG(0, 6);
	}
}

#endif

#endif		/*		portDMA_IS_AVAILABLE		*/


#endif /* Target checking */
