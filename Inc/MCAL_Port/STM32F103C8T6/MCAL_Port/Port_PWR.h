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


/*
 * Initializes power control peripheral. (Left empty if no initialization needed)
 */
void vPort_PWR_init(void);

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
void vPort_PWR_setPvdThreshold(uint32_t uiVoltage);

/*
 * Enables Programmable voltage detector.
 */
#define vPORT_PWR_ENABLE_PVD()	\
	(	LL_PWR_EnablePVD()	)

/*
 * Disables Programmable voltage detector.
 */
#define vPORT_PWR_DISABLEPVD()	\
	(	LL_PWR_DisablePVD()	)

/*
 * Sets Programmable voltage detector callback.
 */
void vPort_PWR_setPvdCallback(void (*pfCallback)(void*), void* pvParams);










#endif /* COTS_OS_INC_MCAL_PORT_PORT_PWR_H_ */
