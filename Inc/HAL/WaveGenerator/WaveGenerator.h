/*
 * WaveGenerator.h
 *
 *  Created on: Nov 10, 2023
 *      Author: Ali Emad
 */

#ifndef INC_HAL_WAVEGENERATOR_WAVEGENERATOR_H_
#define INC_HAL_WAVEGENERATOR_WAVEGENERATOR_H_

typedef struct{
	/**
	 * 						P U B L I C :
	 **/
	/*
	 * DAC unit and channel used for this object.
	 *
	 * DAC unit must be initialized first.
	 */
	uint8_t ucDacUnitNumber;
	uint8_t ucDacChannelNumber;

	/*
	 * "uiGenDiv" is result of the following division:
	 * 		wave generator sample frequency / this wave sample frequency
	 */
	uint32_t uiGenDiv;

	/**
	 * 						P R I V A T E :
	 **/
	/*
	 * Data to be outputted.
	 * "pucDataArr[i]" is an 8-bit DAC word.
	 */
	uint8_t* pucDataArr;
	uint32_t uiDataLen;
	uint32_t uiDataDoneLen;

	StaticSemaphore_t xDoneSemaphoreStatic;
	SemaphoreHandle_t xDoneSemaphore;
}xHOS_WaveGen_Wave_t;

typedef struct{
	/**
	 * 						P U B L I C :
	 **/
	/*
	 * Timer unit number.
	 *
	 * This timer is locked for this waveGen object, and should not be used by any
	 * other SW.
	 */
	uint8_t ucTimerUnitNumber;

	/*
	 * Array of wave objects to be played by this waveGen.
	 */
	xHOS_WaveGen_Wave_t** ppxWaveArr;
	uint8_t ucNWaves;

	/**
	 * 						P R I V A T E :
	 **/
	/*
	 * Counter of how many samples have been done since start of operation.
	 */
	uint32_t uiSampleCnt;
}xHOS_WaveGen_t;


void vHOS_WaveGen_init(xHOS_WaveGen_t* pxHandle, uint32_t uiSampleFreq);

uint8_t ucHOS_WaveGen_blockUntilDone(xHOS_WaveGen_Wave_t* pxHandle, TickType_t xTimeout);

void vHOS_WaveGen_setData(	xHOS_WaveGen_t* pxHandle,
							uint8_t ucWaveIndex,
							uint8_t* pucDataArr,
							uint32_t uiDataLen	);



















#endif /* INC_HAL_WAVEGENERATOR_WAVEGENERATOR_H_ */
