/*
 * Port_EXTI.h
 *
 *  Created on: Jul 26, 2023
 *      Author: Ali Emad
 */

#ifndef COTS_OS_INC_MCAL_PORT_PORT_EXTI_H_
#define COTS_OS_INC_MCAL_PORT_PORT_EXTI_H_

#include "stm32f103xb.h"
#include "stm32f1xx_hal.h"
#include "cmsis_gcc.h"

#include "stm32f1xx_ll_exti.h"
#include "stm32f1xx_ll_gpio.h"

#include "FreeRTOS.h"

extern uint32_t puiPortExtiPinToAfioLineArr[5];
extern uint32_t puiPortExtiPinToExtiLineArr[5];
extern void(*ppfPortExtiCallbackArr[5])(void*);
extern void* ppvPortExtiCallbackParamsArr[5];

/*******************************************************************************
 * API functions:
 ******************************************************************************/
/*
 * Initializes certain pin to trigger an interrupt on signal edges.
 *
 * Notes:
 * 		-	Pin GPIO configuration must be done independently.
 *
 * 		-	Interrupt controller configuration - if any - must be done independently.
 *
 * 		-	Some devices (Like STM32F103) have multiplexed edge detection units.
 * 			Hence, be careful when initializing new lines, as they may overwrite
 * 			a previous initialization of another line.
 *
 * 		-	"ucEdge":
 * 			-	0==> Negative edge.
 * 			-	1==> Positive edge.
 * 			-	2==> Both edges.
 *
 * 		-	If the ported target does not support some pins, port writer must
 * 			declare them in this comment note, and range check them in the function.
 * 			(porting for STM32F103 using this SW gives the ability of using pins 0-4)
 */
static inline void vPort_EXTI_initLine(	uint8_t ucPort,
										uint8_t ucPin,
										uint8_t ucEdge	)
{
	configASSERT(ucPin < 5);

	/*	Map line to the given port	*/
	LL_GPIO_AF_SetEXTISource(ucPort, puiPortExtiPinToAfioLineArr[ucPin]);

	/*	Set triggering edge	*/
	configASSERT(ucEdge < 3);
	switch(ucEdge)
	{
	case 0:
		LL_EXTI_EnableFallingTrig_0_31(puiPortExtiPinToExtiLineArr[ucPin]);
		break;
	case 1:
		LL_EXTI_EnableRisingTrig_0_31(puiPortExtiPinToExtiLineArr[ucPin]);
		break;
	case 2:
		LL_EXTI_EnableFallingTrig_0_31(puiPortExtiPinToExtiLineArr[ucPin]);
		LL_EXTI_EnableRisingTrig_0_31(puiPortExtiPinToExtiLineArr[ucPin]);
		break;
	}
}

/*	Enables line	*/
static inline void vPort_EXTI_enableLine(uint8_t ucPort, uint8_t ucPin)
{
	configASSERT(ucPin < 5);

	LL_EXTI_EnableIT_0_31(puiPortExtiPinToExtiLineArr[ucPin]);
}

/*	Disables line	*/
static inline void vPort_EXTI_disableLine(uint8_t ucPort, uint8_t ucPin)
{
	configASSERT(ucPin < 5);

	LL_EXTI_DisableIT_0_31(puiPortExtiPinToExtiLineArr[ucPin]);
}

/*
 * Clears pending flag.
 *
 * Notes:
 * 		-	This macro does not perform range checking on the parameters for
 * 			fast use, hence must be used with care.
 */
#define vPORT_EXTI_CLEAR_PENDING_FLAG(ucPort, ucPin)	\
	(LL_EXTI_ClearFlag_0_31(puiPortExtiPinToExtiLineArr[(ucPin)]))

/*	Sets callback of EXTI handler	*/
static inline void vPort_EXTI_setCallback(	uint8_t ucPort,
											uint8_t ucPin,
											void(*pfCallback)(void*),
											void* pvParams)
{
	configASSERT(ucPin < 5);

	ppfPortExtiCallbackArr[ucPin] = pfCallback;
	ppvPortExtiCallbackParamsArr[ucPin] = pvParams;
}















#endif /* COTS_OS_INC_MCAL_PORT_PORT_EXTI_H_ */
