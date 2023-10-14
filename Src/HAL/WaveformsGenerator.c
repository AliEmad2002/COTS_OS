/*
 * WaveformsGenerator.c
 *
 *  Created on: Sep 28, 2023
 *      Author: Ali Emad
 */

/*	LIB	*/
#include "stdint.h"
#include "stdio.h"
#include "LIB/Assert.h"
#include "LIB/RunTimeStackAlalyzer/RunTimeStackAlalyzer.h"

/*	RTOS	*/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "RTOS_PRI_Config.h"

/*	MCAL	*/
#include "MCAL_Port/Port_DAC.h"
#include "MCAL_Port/Port_GPIO.h"

/*	HAL	*/
#include "HAL/HWTime/HWTime.h"

/*	SELF	*/
#include "HAL/WaveformsGenerator/WaveformsGenerator.h"


/*******************************************************************************
 * Helping functions:
 ******************************************************************************/
uint32_t uiGetFastest(xHOS_WaveformsGenerator_t* pxGen)
{
	/*
	 * As number of waves would probably be less than 16 (maximum number of DAC
	 * channels in most used targets, linear search is a better approach, rather
	 * than using a code size-consuming algorithm (like heap, pri-queue, etc...)
	 */
	uint32_t uiMinTime = pxGen->ppxWaveArr[0]->uiSampleTime;
	uint32_t uiFastestIndex = 0;

	for (uint32_t i = 0; i < pxGen->uiNumberOfWaves; i++)
	{
		if (pxGen->ppxWaveArr[i]->uiSampleTime < uiMinTime)
		{
			uiMinTime = pxGen->ppxWaveArr[i]->uiSampleTime;
			uiFastestIndex = i;
		}
	}

	return uiFastestIndex;
}


/*******************************************************************************
 * RTOS task function:
 ******************************************************************************/
static void vTask(void* pvParams)
{
	xHOS_WaveformsGenerator_t* pxGen = (xHOS_WaveformsGenerator_t*)pvParams;

	xHOS_WaveformsGenerator_Wave_t* pxWave;

	uint32_t i;

	uint64_t ulCurrentTime, ulDelayEndTime;

	int32_t iVoltage;

	uint32_t uiFastestSampleTime;

	while(1)
	{
		for (i = 0; i < pxGen->uiNumberOfWaves; i++)
		{
			pxWave = pxGen->ppxWaveArr[i];

			ulCurrentTime = ulHOS_HWTime_TICKS_TO_US(ulHOS_HWTime_getTimestamp());

			/*	If it is time for a new sample on the i-th wave	*/
			if ((uint32_t)(ulCurrentTime - pxWave->ulPrevSampleTime) >= pxWave->uiSampleTime)
			{
				/*	Get new sample	*/
				iVoltage = pxWave->pfGetNewSample(ulCurrentTime, pxWave->pvGetNewSampleParams);

				/*	Update DAC channel	*/
				vPort_DAC_setChannelVoltage(	pxWave->ucDacUnitNumber,
												pxWave->ucDacChannelNumber,
												iVoltage	);

				/*	Update prevSampleTime	*/
				pxWave->ulPrevSampleTime = ulCurrentTime;
			}
		}

		uiFastestSampleTime = pxGen->ppxWaveArr[pxGen->uiFastestIndex]->uiSampleTime;

		/*	If sample time of the fastest signal is "portMAX_DELAY".	*/
		if (uiFastestSampleTime == portMAX_DELAY)
		{
			/*	Block until a new fastest sample time exist	*/
			vTaskSuspend(pxGen->xTask);
		}


		/*	Otherwise, delay for update time of the fastest wave	*/
		else
		{
			ulDelayEndTime =
				pxGen->ppxWaveArr[pxGen->uiFastestIndex]->ulPrevSampleTime +
				uiFastestSampleTime	;

			if (ulDelayEndTime > ulCurrentTime)
			{
				vHOS_HardwareDelay_delayUs(
					&pxGen->xHWDelay,
					ulDelayEndTime - ulCurrentTime	);
				//vTaskDelay(0);
			}

			else
			{
				/*	Signals are too fast to handle	*/
			}
		}
	}
}


/*******************************************************************************
 * API functions:
 ******************************************************************************/
/*
 * See header for info.
 */
void vHOS_WaveformsGenerator_init(	xHOS_WaveformsGenerator_t* pxHandle,
									uint8_t ucTimerUnitNumber	)
{
	/*	Initialize waves array	*/
	pxHandle->uiNumberOfWaves = 0;
	pxHandle->uiFastestIndex = 0;

	/*	Initialize HW-delay	*/
	pxHandle->xHWDelay.ucTimerUnitNumber = ucTimerUnitNumber;
	vHOS_HardwareDelay_init(&pxHandle->xHWDelay, xHOS_HardwareDelay_Frequency_500kHz);

	/*	Initialize waveArrMutex	*/
	pxHandle->xWaveArrMutex = xSemaphoreCreateMutexStatic(&pxHandle->xWaveArrMutexStatic);
	xSemaphoreGive(pxHandle->xWaveArrMutex);

	/*	Initialize task	*/
	static uint8_t ucCreatedObjectsCount = 0;
	char pcTaskName[configMAX_TASK_NAME_LEN];
	sprintf(pcTaskName, "WFG%d", ucCreatedObjectsCount++);

	pxHandle->xTask = xTaskCreateStatic(	vTask,
											pcTaskName,
											uiWAVEFORMSGENERATOR_STACK_SIZE,
											(void*)pxHandle,
											configHOS_MID_REAL_TIME_TASK_PRI,
											pxHandle->xTaskStack,
											&pxHandle->xTaskStatic	);

#if ucCONF_RUN_TIME_STACK_ALALYZER_ENABLE
	vLIB_RunTimeStackAlalyzer_addTask(pxHandle->xTask);
#endif
}

/*
 * See header for info.
 */
void vHOS_WaveformsGenerator_initWave(	xHOS_WaveformsGenerator_Wave_t* pxHandle,
										uint32_t uiSampleTime	)
{
	/*	Initialize timing variables	*/
	pxHandle->uiSampleTime = uiSampleTime;
	pxHandle->ulPrevSampleTime = 0;

	/*	Initialize DAC channel	*/
	vPort_DAC_initChannel(pxHandle->ucDacUnitNumber, pxHandle->ucDacChannelNumber);
	vPort_GPIO_initDacChannelPinAsOutput(pxHandle->ucDacUnitNumber, pxHandle->ucDacChannelNumber, 0);
}

/*
 * See header for info.
 */
uint32_t uiHOS_WaveformsGenerator_add(	xHOS_WaveformsGenerator_t* pxGen,
										xHOS_WaveformsGenerator_Wave_t* pxWave	)
{
	/*	Check if generator has enough space	*/
	vLib_ASSERT(pxGen->uiNumberOfWaves < uiWAVEFORMSGENERATOR_MAX_NUMBER_OF_WAVES, 1);

	/*	Lock the wave array	*/
	xSemaphoreTake(pxGen->xWaveArrMutex, portMAX_DELAY);

	/*	If this is the first wave added to the generator, it is considered the fastest	*/
	if (pxGen->uiNumberOfWaves == 0)
		pxGen->uiFastestIndex = 0;

	/*
	 * Otherwise, if other waves were previously added to the generator, check
	 * if the new wave is faster than the fastest wave. If so, update generator's
	 * "uiFastestIndex".
	 */
	else if (pxWave->uiSampleTime < pxGen->ppxWaveArr[pxGen->uiFastestIndex]->uiSampleTime)
	{
		pxGen->uiFastestIndex = pxGen->uiNumberOfWaves;
		vTaskResume(pxGen->xTask);
	}

	/*	Add to the array	*/
	pxGen->ppxWaveArr[pxGen->uiNumberOfWaves] = pxWave;

	/*	Unlock the wave array	*/
	xSemaphoreGive(pxGen->xWaveArrMutex);

	return pxGen->uiNumberOfWaves++;
}

/*
 * See header for info.
 */
void vHOS_WaveformsGenerator_editSampleTime(	xHOS_WaveformsGenerator_t* pxGen,
												uint32_t uiWaveIndex,
												uint32_t uiNewSampleTime	)
{
	/*	Lock the wave array	*/
	xSemaphoreTake(pxGen->xWaveArrMutex, portMAX_DELAY);

	/*	Take copy of the fastest sample time	*/
	uint32_t uiFastestSampleTime = pxGen->ppxWaveArr[pxGen->uiFastestIndex]->uiSampleTime;

	/*	Update sample time	*/
	pxGen->ppxWaveArr[uiWaveIndex]->uiSampleTime = uiNewSampleTime;

	/*
	 * If the new sample time is faster than the fastest wave, update generator's
	 * "uiFastestIndex".
	 */
	if (uiNewSampleTime < uiFastestSampleTime)
	{
		pxGen->uiFastestIndex = uiWaveIndex;
		vTaskResume(pxGen->xTask);
	}

	/*
	 * Otherwise, if the wave to be updated itself is the fastest in the generator,
	 * and got updated with a slower value, search for a new "fastest".
	 */
	else if (uiWaveIndex == pxGen->uiFastestIndex)
		pxGen->uiFastestIndex = uiGetFastest(pxGen);

	/*	Unlock the wave array	*/
	xSemaphoreGive(pxGen->xWaveArrMutex);
}












