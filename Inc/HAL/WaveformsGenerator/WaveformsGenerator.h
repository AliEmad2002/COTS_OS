/*
 * WaveformsGenerator.h
 *
 *  Created on: Sep 28, 2023
 *      Author: Ali Emad
 *
 * When tested on STM32F103C8T6 @72MHz:
 * 		-	Fastest possible sample time = 100us.
 * 		-	CPU time consumption at fastest sample time = 33%
 *
 * TODO: for generating faster signals, and consume less CPU time as well,
 * a better approach would be buffering samples to the DAC using DMA.
 */

#ifndef COTS_OS_INC_HAL_WAVEFORMSGENERATOR_WAVEFORMSGENERATOR_H_
#define COTS_OS_INC_HAL_WAVEFORMSGENERATOR_WAVEFORMSGENERATOR_H_

#define uiWAVEFORMSGENERATOR_STACK_SIZE				configMINIMAL_STACK_SIZE
#define uiWAVEFORMSGENERATOR_MAX_NUMBER_OF_WAVES	2

#include "HAL/HardwareDelay/HardwareDelay.h"

typedef struct{
	/*		PUBLIC		*/
	uint8_t ucDacUnitNumber;
	uint8_t ucDacChannelNumber;

	void* pvGetNewSampleParams;
	int32_t (*pfGetNewSample)(uint64_t, void*);

	/*		PRIVATE		*/
	uint32_t uiSampleTime;
	uint64_t ulPrevSampleTime;
}xHOS_WaveformsGenerator_Wave_t;

typedef struct{
	/*		PRIVATE		*/
	xHOS_WaveformsGenerator_Wave_t* ppxWaveArr[uiWAVEFORMSGENERATOR_MAX_NUMBER_OF_WAVES];
	uint32_t uiNumberOfWaves;
	uint32_t uiFastestIndex;

	xHOS_HardwareDelay_t xHWDelay;

	SemaphoreHandle_t xWaveArrMutex;
	StaticSemaphore_t xWaveArrMutexStatic;

	TaskHandle_t xTask;
	StaticTask_t xTaskStatic;
	StackType_t xTaskStack[uiWAVEFORMSGENERATOR_STACK_SIZE];
}xHOS_WaveformsGenerator_t;

/*
 * Initializes waveforms generator handle.
 */
void vHOS_WaveformsGenerator_init(	xHOS_WaveformsGenerator_t* pxHandle,
									uint8_t ucTimerUnitNumber	);

/*
 * Initializes a wave handle.
 */
void vHOS_WaveformsGenerator_initWave(	xHOS_WaveformsGenerator_Wave_t* pxHandle,
										uint32_t uiSampleTime	);

/*
 * Adds a wave to a waveforms generator.
 *
 * Returns index to the added wave in generator's wave array. (Index is used later
 * to fasten other functions and save searching time)
 */
uint32_t uiHOS_WaveformsGenerator_add(	xHOS_WaveformsGenerator_t* pxGen,
										xHOS_WaveformsGenerator_Wave_t* pxWave	);

/*
 * Edits sample time of a wave.
 */
void vHOS_WaveformsGenerator_editSampleTime(	xHOS_WaveformsGenerator_t* pxGen,
												uint32_t uiWaveIndex,
												uint32_t uiNewSampleTime	);










#endif /* COTS_OS_INC_HAL_WAVEFORMSGENERATOR_WAVEFORMSGENERATOR_H_ */
