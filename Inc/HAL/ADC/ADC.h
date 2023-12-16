/*
 * ADC.h
 *
 *  Created on: Dec 16, 2023
 *      Author: Ali Emad
 */

#ifndef COTS_OS_INC_HAL_ADC_ADC_H_
#define COTS_OS_INC_HAL_ADC_ADC_H_

#include "HAL/ADC/ADC_Config.h"


/*
 * Initializes SW driver.
 */
void vHOS_ADC_init(void);

/*
 * Locks ADC unit.
 *
 * Notes:
 * 		-	It is user's responsibilty to not use or modify unit's configuration
 * 			until locked.
 */
uint8_t ucHOS_ADC_lockUnit(uint8_t ucUnitNumber, TickType_t xTimeout);

/*
 * Unlocks ADC unit.
 *
 * Notes:
 * 		-	This function must be called only by the task which've previously locked
 * 			the ADC unit. (Mutex concept)
 */
void vHOS_ADC_unlockUnit(uint8_t ucUnitNumber);

/*
 * Selects which channel of the analog channels should be connected to the ADC unit.
 */
void vHOS_ADC_selectChannel(uint8_t ucUnitNumber, uint8_t ucChannelNumber);

/*
 * Triggers ADC sample and conversion.
 */
void vHOS_ADC_triggerRead(uint8_t ucUnitNumber);

/*
 * Selects whether ADC works in continuous or single sample mode.
 *
 * Notes:
 * 		-	"ucMode": 0 ==> Continuous, 1 ==> Single sample.
 */
void vHOS_ADC_selectMode(uint8_t ucUnitNumber, uint8_t ucMode);

/*
 * Blocks until new ADC conversion is done, or timeout passes.
 */
uint8_t ucHOS_ADC_blockUntilEOC(uint8_t ucUnitNumber, TickType_t xTimeout);

/*
 * Read latest ADC conversion result. (Numerical value, not voltage)
 */
uint32_t uiHOS_ADC_read(uint8_t ucUnitNumber);

/*
 * Macro expressing "v_ref" of an ADC unit (in mV).
 *
 * Notes:
 * 		-	"v_ref" values of multiple ADC units is configured in "ADC_Config.h" file.
 */
#define uiHOS_ADC_GET_V_REF_MV(ucUnitNumber)	\
	(/*	TODO	*/)

/*
 * Macro expressing "max_read" (i.e.: 2^Number_of_bits) of an ADC unit.
 *
 * Notes:
 * 		-	"max_read" values of multiple ADC units is configured in "ADC_Config.h" file.
 */
#define uiHOS_ADC_GET_MAX_READ(ucUnitNumber)	\
	(/*	TODO	*/)



#endif /* COTS_OS_INC_HAL_ADC_ADC_H_ */
