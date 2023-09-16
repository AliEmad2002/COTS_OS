/*
 * Port_PWR.h
 *
 *  Created on: Sep 15, 2023
 *      Author: Ali Emad
 */

#ifndef COTS_OS_INC_MCAL_PORT_PORT_PWR_H_
#define COTS_OS_INC_MCAL_PORT_PORT_PWR_H_

#include "stm32f1xx.h"
#include "stm32f1xx_ll_pwr.h"
#include "stm32f1xx_ll_exti.h"

#include "LIB/Assert.h"


extern void(*pfPortPwrPvdCallback)(void*);
extern void* ppvPortPwrPvdCallbackParams;

/*
 * Initializes power control peripheral. (Left empty if no initialization needed)
 */
static inline void vPort_PWR_init(void)
{
	/*
	 * Initialize Programmable voltage detector EXTI line to generate an interrupt
	 * on VDD falling edge (PVD rising edge).
	 */
	LL_EXTI_EnableRisingTrig_0_31(LL_EXTI_LINE_16);
	LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_16);
}

/*
 * Programmable voltage detector threshold selection.
 *
 * Notes:
 * 		-	Programmable voltage detector (PVD) is useful for detecting a falling
 * 			edge on the main supply voltage. Which helps in predicting a sudden
 * 			shut-down event and executing a shut-down routine very fast before
 * 			the power is completely gone.
 *
 * 		-	If this feature is not available in the used target, it could be emulated
 * 			using ADC, or ignored if it won't be used in the upper layers' code.
 *
 * 		-	"uiVoltage": is threshold voltage in mV.
 */
static inline void vPort_PWR_setPvdThreshold(uint32_t uiVoltage)
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

/*
 * Enables Programmable voltage detector.
 */
static inline void vPort_PWR_enablePvd(void)
{
	LL_PWR_EnablePVD();
}

/*
 * Disables Programmable voltage detector.
 */
static inline void vPort_PWR_disablePvd(void)
{
	LL_PWR_DisablePVD();
}

/*
 * Sets Programmable voltage detector callback.
 */
static inline void vPort_PWR_setPvdCallback(void (*pfCallback)(void*), void* pvParams)
{
	pfPortPwrPvdCallback = pfCallback;
	ppvPortPwrPvdCallbackParams = pvParams;
}










#endif /* COTS_OS_INC_MCAL_PORT_PORT_PWR_H_ */
