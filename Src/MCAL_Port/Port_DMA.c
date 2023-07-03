///*
// * Port_DMA.c
// *
// *  Created on: Jun 17, 2023
// *      Author: Ali Emad
// */
//
//#include "stm32f1xx.h"
//#include "MCAL_Port/Port_DMA.h"
//
///*
// * This is a multidimensional array of pointers to DMA channels.
// * First dimension specifies DMA unit number, and the second specifies DMA channel
// * number.
// */
//DMA_Channel_TypeDef* const pxPort_DmaChArr[1][7] = {
//	{
//		DMA1_Channel1, DMA1_Channel2, DMA1_Channel3, DMA1_Channel4,
//		DMA1_Channel5, DMA1_Channel6, DMA1_Channel7
//	}
//};
//
///*
// * Array to DMA peripherals.
// */
//DMA_TypeDef* const pxPort_DmaArr[] = {DMA1};
//
///*******************************************************************************
// * Helping functions:
// ******************************************************************************/
//
///*******************************************************************************
// * Functions:
// ******************************************************************************/
//void vPort_DMA_startTransfer(xPort_DMA_TransInfo_t* pxInfo)
//{
//	DMA_Channel_TypeDef* pxChHandle = pxPort_DmaChArr[pxInfo->ucUnitNumber][pxInfo->ucChannelNumber];
//
//	/*	Disable channel	*/
//	vCLR_BIT(pxChHandle->CCR, 0);
//
//	/*	Temporarily store CCR value in a register	*/
//	uint32_t uiCCR = pxChHandle->CCR;
//
//	/*	Set transfer direction	*/
//	if (pxInfo->ucDirection == 0)
//		vCLR_BIT(uiCCR, 4);
//	else
//		vSET_BIT(uiCCR, 4);
//
//	/*	Write peripheral increment setting	*/
//	if (pxInfo->ucPeripheralIncrement == 0)
//		vCLR_BIT(uiCCR, 6);
//	else
//		vSET_BIT(uiCCR, 6);
//
//	/*	Write memory increment setting	*/
//	if (pxInfo->ucMemoryIncrement == 0)
//		vCLR_BIT(uiCCR, 7);
//	else
//		vSET_BIT(uiCCR, 7);
//
//	/*	Write peripheral data unit size	*/
//	vEDT_REG(uiCCR, 8, pxInfo->xPeripheralSize, 2);
//
//	/*	Write memory data unit size	*/
//	vEDT_REG(uiCCR, 10, pxInfo->xMemorySize, 2);
//
//	/*	Write priority setting	*/
//	vEDT_REG(uiCCR, 12, pxInfo->ucPriLevel, 2);
//
//	/*	Write trigger source	*/
//	if (pxInfo->ucTriggerSource == 0)
//		vCLR_BIT(uiCCR, 14);
//	else
//		vSET_BIT(uiCCR, 14);
//
//	/*	Write temporary register to actual register	*/
//	pxChHandle->CCR = uiCCR;
//
//	/*	Write number of data unit to be transferred	*/
//	pxChHandle->CNDTR = pxInfo->uiN;
//
//	/*	Write starting addresses	*/
//	pxChHandle->CPAR = pxInfo->pvPeripheralStartingAdderss;
//	pxChHandle->CMAR = pxInfo->pvMemoryStartingAdderss;
//
//	/*	Enable channel	*/
//	vSET_BIT(pxChHandle->CCR, 0);
//}
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
