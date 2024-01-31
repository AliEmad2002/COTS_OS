/*
 * Port_Interrupt.c
 *
 *  Created on: Jun 13, 2023
 *      Author: Ali Emad
 */

/*	Target checking	*/
#include "MCAL_Port/Port_Target.h"
#ifdef MCAL_PORT_TARGET_STM32F401RCT6


#include "stm32f401xc.h"
#include "stm32f4xx_hal.h"
#include "cmsis_gcc.h"

const IRQn_Type pxPortInterruptSpiTxeIrqNumberArr[] = {
	SPI1_IRQn, SPI2_IRQn, SPI3_IRQn
};

const IRQn_Type* pxPortInterruptSpiRxneIrqNumberArr = pxPortInterruptSpiTxeIrqNumberArr;

const IRQn_Type pxPortInterruptTimerOvfIrqNumberArr[] = {
	TIM1_UP_TIM10_IRQn
};

const IRQn_Type* pxPortInterruptTimerCcIrqNumberArr = pxPortInterruptTimerOvfIrqNumberArr;

const IRQn_Type pxPortInterruptDma1IrqNumberArr[] = {
	DMA1_Stream0_IRQn, DMA1_Stream1_IRQn, DMA1_Stream2_IRQn,
	DMA1_Stream3_IRQn, DMA1_Stream4_IRQn, DMA1_Stream5_IRQn,
	DMA1_Stream6_IRQn, DMA1_Stream7_IRQn
};

const IRQn_Type pxPortInterruptDma2IrqNumberArr[] = {
	DMA2_Stream0_IRQn, DMA2_Stream1_IRQn, DMA2_Stream2_IRQn,
	DMA2_Stream3_IRQn, DMA2_Stream4_IRQn, DMA2_Stream5_IRQn,
	DMA2_Stream6_IRQn, DMA2_Stream7_IRQn
};

const IRQn_Type* ppxPortInterruptDmaIrqNumberArr[] = {
		pxPortInterruptDma1IrqNumberArr,
		pxPortInterruptDma2IrqNumberArr
};







#endif /* Target checking */