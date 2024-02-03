/*
 * Port_PWR.c
 *
 *  Created on: Sep 15, 2023
 *      Author: Ali Emad
 */

/*	Target checking	*/
#include "MCAL_Port/Port_Target.h"
#ifdef MCAL_PORT_TARGET_STM32F401RCT6


#include "LIB/Assert.h"

#include "stm32f4xx.h"
#include "stm32f4xx_ll_exti.h"

#include "MCAL_Port/Port_Interrupt.h"
#include "MCAL_Port/Port_PWR.h"


void vPort_PWR_init(void)
{
	/*
	 * Initialize Programmable voltage detector EXTI line to generate an interrupt
	 * on VDD falling edge (PVD rising edge).
	 */
	LL_EXTI_EnableRisingTrig_0_31(LL_EXTI_LINE_16);
	LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_16);
}

void vPort_PWR_setPvdThreshold(uint32_t uiVoltage)
{
	/*	Available range in STM32F103C8T6 is: 2.2V - 2.9V	*/
	vLib_ASSERT(2200 <= uiVoltage && uiVoltage <= 2900, 0);

	if (uiVoltage < 2250)		LL_PWR_SetPVDLevel(LL_PWR_PVDLEVEL_0);
	else if (uiVoltage < 2350)	LL_PWR_SetPVDLevel(LL_PWR_PVDLEVEL_1);
	else if (uiVoltage < 2450)	LL_PWR_SetPVDLevel(LL_PWR_PVDLEVEL_2);
	else if (uiVoltage < 2550)	LL_PWR_SetPVDLevel(LL_PWR_PVDLEVEL_3);
	else if (uiVoltage < 2650)	LL_PWR_SetPVDLevel(LL_PWR_PVDLEVEL_4);
	else if (uiVoltage < 2750)	LL_PWR_SetPVDLevel(LL_PWR_PVDLEVEL_5);
	else if (uiVoltage < 2850)	LL_PWR_SetPVDLevel(LL_PWR_PVDLEVEL_6);
	else 						LL_PWR_SetPVDLevel(LL_PWR_PVDLEVEL_7);
}


#ifdef ucPORT_INTERRUPT_IRQ_DEF_PWR_PVD

void(*pfPortPwrPvdCallback)(void*);
void* ppvPortPwrPvdCallbackParams;

void vPort_PWR_setPvdCallback(void (*pfCallback)(void*), void* pvParams)
{
	pfPortPwrPvdCallback = pfCallback;
	ppvPortPwrPvdCallbackParams = pvParams;
}

void PVD_IRQHandler(void)
{
	pfPortPwrPvdCallback(ppvPortPwrPvdCallbackParams);
	LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_16);
}

#endif


#endif /* Target checking */
