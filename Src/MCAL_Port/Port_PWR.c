/*
 * Port_PWR.c
 *
 *  Created on: Sep 15, 2023
 *      Author: Ali Emad
 */

#include "stm32f1xx.h"
#include "stm32f1xx_ll_exti.h"

#include "MCAL_Port/Port_Interrupt.h"

void(*pfPortPwrPvdCallback)(void*);
void* ppvPortPwrPvdCallbackParams;


#ifdef ucPORT_INTERRUPT_IRQ_DEF_PWR_PVD
void PVD_IRQHandler(void)
{
	pfPortPwrPvdCallback(ppvPortPwrPvdCallbackParams);
	LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_16);
}
#endif
