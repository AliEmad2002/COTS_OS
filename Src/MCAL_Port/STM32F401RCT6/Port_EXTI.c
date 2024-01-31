/*
 * Port_EXTI.c
 *
 *  Created on: Jul 26, 2023
 *      Author: Ali Emad
 */

/*	Target checking	*/
#include "MCAL_Port/Port_Target.h"
#ifdef MCAL_PORT_TARGET_STM32F401RCT6


#include "stm32f401xc.h"
#include "stm32f4xx_hal.h"
#include "cmsis_gcc.h"
#include "stm32f4xx_ll_system.h"
#include "stm32f4xx_ll_exti.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_hal_gpio.h"

#include "LIB/Assert.h"

#include "MCAL_Port/Port_Interrupt.h"

const uint32_t puiPortExtiPinToSyscfgLineArr[16] = {
	LL_SYSCFG_EXTI_LINE0 ,
	LL_SYSCFG_EXTI_LINE1 ,
	LL_SYSCFG_EXTI_LINE2 ,
	LL_SYSCFG_EXTI_LINE3 ,
	LL_SYSCFG_EXTI_LINE4 ,
	LL_SYSCFG_EXTI_LINE5 ,
	LL_SYSCFG_EXTI_LINE6 ,
	LL_SYSCFG_EXTI_LINE7 ,
	LL_SYSCFG_EXTI_LINE8 ,
	LL_SYSCFG_EXTI_LINE9 ,
	LL_SYSCFG_EXTI_LINE10 ,
	LL_SYSCFG_EXTI_LINE11 ,
	LL_SYSCFG_EXTI_LINE12 ,
	LL_SYSCFG_EXTI_LINE13 ,
	LL_SYSCFG_EXTI_LINE14 ,
	LL_SYSCFG_EXTI_LINE15
};

const uint32_t puiPortExtiPinToExtiLineArr[16] = {
		LL_EXTI_LINE_0 ,
		LL_EXTI_LINE_1 ,
		LL_EXTI_LINE_2 ,
		LL_EXTI_LINE_3 ,
		LL_EXTI_LINE_4 ,
		LL_EXTI_LINE_5 ,
		LL_EXTI_LINE_6 ,
		LL_EXTI_LINE_7 ,
		LL_EXTI_LINE_8 ,
		LL_EXTI_LINE_9 ,
		LL_EXTI_LINE_10 ,
		LL_EXTI_LINE_11 ,
		LL_EXTI_LINE_12 ,
		LL_EXTI_LINE_13 ,
		LL_EXTI_LINE_14 ,
		LL_EXTI_LINE_15
};

#include "MCAL_Port/Port_EXTI.h"

extern GPIO_TypeDef* const pxPortDioPortArr[];

/*******************************************************************************
 * API functions:
 ******************************************************************************/
void vPort_EXTI_setEdge(uint8_t ucPort, uint8_t ucPin, uint8_t ucEdge)
{
	/*	Map line to the given port	*/
	LL_SYSCFG_SetEXTISource(ucPort, puiPortExtiPinToSyscfgLineArr[ucPin]);

	/*	Set triggering edge	*/
	configASSERT(ucEdge < 3);
	switch(ucEdge)
	{
	case 0:
		LL_EXTI_EnableFallingTrig_0_31(puiPortExtiPinToExtiLineArr[ucPin]);
		LL_EXTI_DisableRisingTrig_0_31(puiPortExtiPinToExtiLineArr[ucPin]);
		break;
	case 1:
		LL_EXTI_EnableRisingTrig_0_31(puiPortExtiPinToExtiLineArr[ucPin]);
		LL_EXTI_DisableFallingTrig_0_31(puiPortExtiPinToExtiLineArr[ucPin]);
		break;
	case 2:
		LL_EXTI_EnableFallingTrig_0_31(puiPortExtiPinToExtiLineArr[ucPin]);
		LL_EXTI_EnableRisingTrig_0_31(puiPortExtiPinToExtiLineArr[ucPin]);
		break;
	}
}

uint8_t ucPort_EXTI_getEdge(uint8_t ucPort, uint8_t ucPin)
{
	uint8_t ucRising = LL_EXTI_IsEnabledRisingTrig_0_31(puiPortExtiPinToExtiLineArr[ucPin]);
	uint8_t ucFalling = LL_EXTI_IsEnabledFallingTrig_0_31(puiPortExtiPinToExtiLineArr[ucPin]);

	if (ucRising && ucFalling)
		return 2;
	else if (ucRising)
		return 1;
	else
		return 0;
}

/*
 * TODO:
 * this function should deprecate the "pxPortInterruptExtiIrqNumberArr[]" array.
 */
uint32_t uiPort_EXTI_getIrqNum(uint8_t ucPort, uint8_t ucPin)
{
	if (ucPin < 5)							return EXTI0_IRQn + ucPin;
	else if (5 <= ucPin && ucPin <= 9)		return EXTI9_5_IRQn;
	else if (10 <= ucPin && ucPin <= 15)	return EXTI15_10_IRQn;

	else
	{
		vLib_ASSERT(0, 0);
		return 0;
	}
}

/*******************************************************************************
 * ISRs:
 *
 * Notes
 * 		-	Define them as shown, target dependent.
 * 		-	Add clearing pending flag to the end of the ISR
 ******************************************************************************/
#ifdef ucPORT_INTERRUPT_IRQ_DEF_EXTI

void(*ppfPortExtiCallbackArr[16])(void*);
void* ppvPortExtiCallbackParamsArr[16];

void vPort_EXTI_setCallback(	uint8_t ucPort,
											uint8_t ucPin,
											void(*pfCallback)(void*),
											void* pvParams)
{
	ppfPortExtiCallbackArr[ucPin] = pfCallback;
	ppvPortExtiCallbackParamsArr[ucPin] = pvParams;
}

void EXTI0_IRQHandler(void)
{
	ppfPortExtiCallbackArr[0](ppvPortExtiCallbackParamsArr[0]);
	LL_EXTI_ClearFlag_0_31(puiPortExtiPinToExtiLineArr[0]);
	__asm volatile( "dsb" ::: "memory" );
	__asm volatile( "isb" );
}

void EXTI1_IRQHandler(void)
{
	ppfPortExtiCallbackArr[1](ppvPortExtiCallbackParamsArr[1]);
	LL_EXTI_ClearFlag_0_31(puiPortExtiPinToExtiLineArr[1]);
	__asm volatile( "dsb" ::: "memory" );
	__asm volatile( "isb" );
}

void EXTI2_IRQHandler(void)
{
	ppfPortExtiCallbackArr[2](ppvPortExtiCallbackParamsArr[2]);
	LL_EXTI_ClearFlag_0_31(puiPortExtiPinToExtiLineArr[2]);
	__asm volatile( "dsb" ::: "memory" );
	__asm volatile( "isb" );
}

void EXTI3_IRQHandler(void)
{
	ppfPortExtiCallbackArr[3](ppvPortExtiCallbackParamsArr[3]);
	LL_EXTI_ClearFlag_0_31(puiPortExtiPinToExtiLineArr[3]);
	__asm volatile( "dsb" ::: "memory" );
	__asm volatile( "isb" );
}

void EXTI4_IRQHandler(void)
{
	ppfPortExtiCallbackArr[4](ppvPortExtiCallbackParamsArr[4]);
	LL_EXTI_ClearFlag_0_31(puiPortExtiPinToExtiLineArr[4]);
	__asm volatile( "dsb" ::: "memory" );
	__asm volatile( "isb" );
}

void EXTI9_5_IRQHandler(void)
{
	for (uint8_t i = 5; i <= 9; i++)
	{
		if (LL_EXTI_ReadFlag_0_31(puiPortExtiPinToExtiLineArr[i]))
		{
			ppfPortExtiCallbackArr[i](ppvPortExtiCallbackParamsArr[i]);
			LL_EXTI_ClearFlag_0_31(puiPortExtiPinToExtiLineArr[i]);
		}
	}
	__asm volatile( "dsb" ::: "memory" );
	__asm volatile( "isb" );
}

void EXTI15_10_IRQHandler(void)
{
	for (uint8_t i = 10; i <= 15; i++)
	{
		if (LL_EXTI_ReadFlag_0_31(puiPortExtiPinToExtiLineArr[i]))
		{
			ppfPortExtiCallbackArr[i](ppvPortExtiCallbackParamsArr[i]);
			LL_EXTI_ClearFlag_0_31(puiPortExtiPinToExtiLineArr[i]);
		}
	}
	__asm volatile( "dsb" ::: "memory" );
	__asm volatile( "isb" );
}
#endif








#endif /* Target checking */
