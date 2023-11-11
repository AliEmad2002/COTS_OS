/*
 * WaveGenerator.c
 *
 *  Created on: Nov 10, 2023
 *      Author: Ali Emad
 */

/*	RTOS	*/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "RTOS_PRI_Config.h"

/*	LIB	*/
#include <stdint.h>

/*	MCAL (Ported)	*/
#include "MCAL_Port/Port_Timer.h"
#include "MCAL_Port/Port_Interrupt.h"
#include "MCAL_Port/Port_DAC.h"

/*	SELF	*/
#include "HAL/WaveGenerator/WaveGenerator.h"

static void vTimOvfCallback(void* pvParams)
{
	xHOS_WaveGen_t* pxHandle = (xHOS_WaveGen_t*)pvParams;
	xHOS_WaveGen_Wave_t* pxWave;
	BaseType_t xHptWoken = pdFALSE;
	uint8_t ucAllWavesAreIdle = 1;

	/*	Loop on all object's waves	*/
	for (uint8_t i = 0; i < pxHandle->ucNWaves; i++)
	{
		pxWave = pxHandle->ppxWaveArr[i];

		/*	If wave object has no data to output, skip it	*/
		if (pxWave->pucDataArr == NULL)
			continue;

		/*	If it is not time for this wave to update its output, skip it	*/
		if (pxHandle->uiSampleCnt % pxWave->uiGenDiv != 0)
			continue;

		/*	Update wave's output	*/
		vPORT_DAC_WRITE_WORD(
			pxWave->ucDacUnitNumber,
			pxWave->ucDacChannelNumber,
			pxWave->pucDataArr[pxWave->uiDataDoneLen]	);

		pxWave->uiDataDoneLen++;

		ucAllWavesAreIdle = 0;

		/*
		 * If not all of wave's data has been outputted, continue to the next
		 * wave (if any)
		 */
		if (pxWave->uiDataDoneLen < pxWave->uiDataLen)
			continue;

		/*
		 * Otherwise, give wave's done semaphore and reset its data pointer
		 */
		pxWave->pucDataArr = NULL;
		xSemaphoreGiveFromISR(pxWave->xDoneSemaphore, &xHptWoken);
	}

	/*	Increment sample counter	*/
	pxHandle->uiSampleCnt++;

	/*	Disable interrupt if all waves are idle (This saves CPU time when no
	 * signals are being generated).
	 */
	if (ucAllWavesAreIdle)
		vPORT_TIM_DISABLE_OVF_INTERRUPT(pxHandle->ucTimerUnitNumber);

	portYIELD_FROM_ISR(xHptWoken);
}

void vHOS_WaveGen_init(xHOS_WaveGen_t* pxHandle, uint32_t uiSampleFreq)
{
	/*	Initialize object linked in wave array	*/
	xHOS_WaveGen_Wave_t* pxWave;
	for (uint8_t i = 0; i < pxHandle->ucNWaves; i++)
	{
		pxWave = pxHandle->ppxWaveArr[i];

		pxWave->pucDataArr = NULL;

		pxWave->xDoneSemaphore =
			xSemaphoreCreateBinaryStatic(&pxWave->xDoneSemaphoreStatic);

		xSemaphoreTake(pxWave->xDoneSemaphore, 0);

		vPORT_DAC_WRITE_WORD(pxWave->ucDacUnitNumber, pxWave->ucDacChannelNumber, 0);
	}

	/*
	 * No samples have been done yet:
	 */
	pxHandle->uiSampleCnt = 0;

	/*
	 * Initialize timer unit to generate an OVF interrupt at "uiSampleFreq" Hz:
	 */
	uiPort_TIM_setOvfFreq(pxHandle->ucTimerUnitNumber, uiSampleFreq);

	vPORT_TIM_ENABLE_OVF_INTERRUPT(pxHandle->ucTimerUnitNumber);

	vPort_TIM_setOvfCallback(
		pxHandle->ucTimerUnitNumber,
		vTimOvfCallback,
		(void*)pxHandle	);

	uint32_t uiIrqNum =
		pxPortInterruptTimerOvfIrqNumberArr[pxHandle->ucTimerUnitNumber];

	VPORT_INTERRUPT_SET_PRIORITY(uiIrqNum, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);

	vPORT_INTERRUPT_ENABLE_IRQ(uiIrqNum);

	vPORT_TIM_ENABLE_COUNTER(pxHandle->ucTimerUnitNumber);
}

uint8_t ucHOS_WaveGen_blockUntilDone(xHOS_WaveGen_Wave_t* pxHandle, TickType_t xTimeout)
{
	return xSemaphoreTake(pxHandle->xDoneSemaphore, xTimeout);
}

void vHOS_WaveGen_setData(	xHOS_WaveGen_t* pxHandle,
							uint8_t ucWaveIndex,
							uint8_t* pucDataArr,
							uint32_t uiDataLen	)
{
	xHOS_WaveGen_Wave_t* pxWave = pxHandle->ppxWaveArr[ucWaveIndex];

	pxWave->pucDataArr = pucDataArr;
	pxWave->uiDataLen = uiDataLen;
	pxWave->uiDataDoneLen = 0;

	vPORT_TIM_ENABLE_OVF_INTERRUPT(pxHandle->ucTimerUnitNumber);
}
























