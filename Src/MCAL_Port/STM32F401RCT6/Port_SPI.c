/*
 * Port_SPI.c
 *
 *  Created on: Jun 13, 2023
 *      Author: Ali Emad
 */

/*	Target checking	*/
#include "MCAL_Port/Port_Target.h"
#ifdef MCAL_PORT_TARGET_STM32F401RCT6


#include "stm32f401xc.h"
#include "stm32f4xx_hal.h"

#include "MCAL_Port/Port_SPI.h"
#include "MCAL_Port/Port_Interrupt.h"

/*******************************************************************************
 * Units.
 ******************************************************************************/
SPI_TypeDef* const pxPortSpiArr[portSPI_NUMBER_OF_UNITS] = {SPI1, SPI2, SPI3};

/*******************************************************************************
 * Callbacks.
 ******************************************************************************/
void (*ppfPortSpiTxeCallbackArr[portSPI_NUMBER_OF_UNITS])(void*);
void* ppvPortSpiTxeCallbackParamsArr[portSPI_NUMBER_OF_UNITS];

void (*ppfPortSpiRxneCallbackArr[portSPI_NUMBER_OF_UNITS])(void*);
void* ppvPortSpiRxneCallbackParamsArr[portSPI_NUMBER_OF_UNITS];

/*******************************************************************************
 * DMA mapping.
 *
 * Notes:
 * 		-	The following array defines mapping of i-th SPI unit's transfer
 * 			complete signal, with a DMA channel such that:
 * 			pxPortSpiDmaMapping[i] = {DmaUnitNumber, DmaChannelNumber}
 *
 * 		-	If the used target has Static SPI/DMA connection, define these
 * 			connections in the following array.
 *
 * 		-	Otherwise, if SPI/DMA connections are dynamic, or there's no DMA in
 * 			the used target, leave this array empty.
 ******************************************************************************/
const uint8_t ppucPortSpiTxeDmaMapping[portSPI_NUMBER_OF_UNITS][2] = {
	{0, 2},
	{0, 4}
};

/*******************************************************************************
 * ISRs:
 *
 * Notes
 * 		-	Define them as shown, target dependent.
 ******************************************************************************/
#ifdef ucPORT_INTERRUPT_IRQ_DEF_SPI
void SPI1_IRQHandler(void)
{
	if(ucPort_SPI_IsTxeInterruptEnabled(0) && ucPORT_SPI_GET_TXE_FLAG(0))
	{
		ppfPortSpiTxeCallbackArr[0](ppvPortSpiTxeCallbackParamsArr[0]);
	}
	if(ucPort_SPI_IsRxneInterruptEnabled(0) && ucPORT_SPI_GET_RXNE_FLAG(0))
	{
		ppfPortSpiRxneCallbackArr[0](ppvPortSpiRxneCallbackParamsArr[0]);
	}
}

void SPI2_IRQHandler(void)
{
	if(ucPort_SPI_IsTxeInterruptEnabled(1) && ucPORT_SPI_GET_TXE_FLAG(1))
	{
		ppfPortSpiTxeCallbackArr[1](ppvPortSpiTxeCallbackParamsArr[1]);
	}
	if(ucPort_SPI_IsRxneInterruptEnabled(1) && ucPORT_SPI_GET_RXNE_FLAG(1))
	{
		ppfPortSpiRxneCallbackArr[1](ppvPortSpiRxneCallbackParamsArr[1]);
	}
}
#endif


























#endif /* Target checking */