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
 * Sets sample time of a channel.
 *
 * Notes:
 * 		-	"uiSampleTime": Is sample time in micro-second, multiplied by ten.
 */
void vHOS_ADC_setSampleTime(	uint8_t ucUnitNumber,
								uint8_t ucChannelNumber,
								uint32_t uiSampleTime	);

/*
 * Selects conversion triggering source (SW or timer)
 *
 * Notes:
 * 		-	"ucSrc": 0==> SW trigger,	1==> Timer trigger.
 * 		-	Timer unit is configured in "pucPortADCTriggeringTimerUnitNumber[]".
 */
void vHOS_ADC_setTriggerSource(uint8_t ucUnitNumber, uint8_t ucSrc);

/*
 * Triggers ADC sample and conversion.
 *
 * Notes:
 * 		-	SW triggering must be initially selected as trigger source.
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
 * Reads certain channel, blocks the calling task until read is done.
 *
 * Notes:
 * 		-	ADC unit should be previously allocated for the calling task, this is
 * 			user's responsibilty.
 */
uint32_t uiHOS_ADC_readChannelBlocking(uint8_t ucUnitNumber, uint8_t ucChannelNumber);

/*
 * Calibrates a "raw" read sample using internal voltage reference. Return value
 * is in micro-Volts.
 */
//todo: make this "uiVrefIntRead" static and updates in a task.
int32_t iHOS_ADC_getVoltageCalib(int32_t iRawRead, uint32_t uiVrefIntRead);

/*
 * Converts ADC reading to uV directly using "uiPORT_ADC_VREF_IN_MV".
 */
uint32_t uiHOS_ADC_getVoltageDirect(uint32_t uiRawRead);

/*
 * Initializes DMA accelerated sampling.
 *
 * Notes:
 * 		-	Not all units or all microcontrollers support DMA accelerated ADC
 * 			sampling. Hence, this function is not 100% portable, and
 * 			"pucPortADCDoesUnitSupportDMA[]" in "port_ADC.h" must be set accordingly.
 *
 * 		-	DMA must be initialized first.
 */
void vHOS_ADC_initDmaSampling(	uint8_t ucUnitNumber,
								uint8_t* pucChannelNumberSequenceArr,
								uint8_t ucChannelSequenceLength
								);

#endif /* COTS_OS_INC_HAL_ADC_ADC_H_ */
