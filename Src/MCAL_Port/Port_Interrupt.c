/*
 * Port_Interrupt.c
 *
 *  Created on: Jun 13, 2023
 *      Author: Ali Emad
 */

#include "stm32f103xb.h"
#include "stm32f1xx_hal.h"
#include "cmsis_gcc.h"

const IRQn_Type pxPortInterruptSpiTxeIrqNumberArr[] = {SPI1_IRQn, SPI2_IRQn};

const IRQn_Type* pxPortInterruptSpiRxneIrqNumberArr = pxPortInterruptSpiTxeIrqNumberArr;

const IRQn_Type pxPortInterruptTimerOvfIrqNumberArr[] = {
	TIM1_UP_IRQn, TIM2_IRQn, TIM3_IRQn, TIM4_IRQn
};

const IRQn_Type* pxPortInterruptTimerCcIrqNumberArr = pxPortInterruptTimerOvfIrqNumberArr;

const IRQn_Type pxPortInterruptDmaIrqNumberArr[] = {
	DMA1_Channel1_IRQn, DMA1_Channel2_IRQn, DMA1_Channel3_IRQn,
	DMA1_Channel4_IRQn, DMA1_Channel5_IRQn, DMA1_Channel6_IRQn,
	DMA1_Channel7_IRQn
};

const IRQn_Type xPortInterruptUsbIrqNumber = USB_LP_CAN1_RX0_IRQn;

const IRQn_Type pxPortInterruptUartTxeIrqNumberArr[] = {
	USART1_IRQn, USART2_IRQn, USART3_IRQn
};

const IRQn_Type* pxPortInterruptUartRxneIrqNumberArr = pxPortInterruptUartTxeIrqNumberArr;

const IRQn_Type* pxPortInterruptUartTcIrqNumberArr = pxPortInterruptUartTxeIrqNumberArr;

const IRQn_Type xPortInterruptPwrPvdIrqNumber = PVD_IRQn;






