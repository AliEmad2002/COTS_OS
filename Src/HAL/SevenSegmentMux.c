/*
 * SevenSegmentMux.c
 *
 *  Created on: Jun 9, 2023
 *      Author: Ali Emad
 */

/*	LIB	*/
#include <stdint.h>
#include "LIB/Bit_Math.h"

/*	FreeRTOS	*/
#include "FreeRTOS.h"
#include "task.h"

/*	MCAL (Ported)	*/
#include "MCAL_Port/Port_DIO.h"
#include "MCAL_Port/Port_Print.h"
#include "MCAL_Port/Port_Breakpoint.h"

/*	HAL-OS	*/
#include "RTOS_PRI_Config.h"

/*	SELF	*/
#include "HAL/SevenSegmentMux/SevenSegmentMux.h"


/*******************************************************************************
 * Private configurations:
 ******************************************************************************/
#define uiSEVEN_SEGMENT_MUX_STACK_SIZE	configMINIMAL_STACK_SIZE

/*******************************************************************************
 * Static objects:
 ******************************************************************************/
static const uint8_t pucSegmentStateArr[] = {
	0b00111111,	/*	0	*/
	0b00000110,	/*	1	*/
	0b01011011,	/*	2	*/
	0b01001111,	/*	3	*/
	0b01100110,	/*	4	*/
	0b01101101,	/*	5	*/
	0b01111101,	/*	6	*/
	0b00000111,	/*	7	*/
	0b01111111,	/*	8	*/
	0b01101111	/*	9	*/
};

/*******************************************************************************
 * Helping functions/macros.
 ******************************************************************************/
static inline void vDisableCurrentlyActiveDigit(xHOS_SevenSegmentMux_t* pxHandle)
{
	uint8_t ucCurrent = pxHandle->ucCurrentActiveDigit;
	vPORT_DIO_WRITE_PIN(	pxHandle->pxDigitEnablePortNumberArr[ucCurrent],
						pxHandle->pxDigitEnablePinNumberArr[ucCurrent],
						!(pxHandle->ucEnableActiveLevel)	);
}

static inline void vEnableCurrentlyActiveDigit(xHOS_SevenSegmentMux_t* pxHandle)
{
	uint8_t ucCurrent = pxHandle->ucCurrentActiveDigit;
	vPORT_DIO_WRITE_PIN(	pxHandle->pxDigitEnablePortNumberArr[ucCurrent],
						pxHandle->pxDigitEnablePinNumberArr[ucCurrent],
						pxHandle->ucEnableActiveLevel	);
}

#define vINCREMENT_CURRENT_ACTIVE_COUNTER(pxHandle)                		    \
{                                                                    		\
	(pxHandle)->ucCurrentActiveDigit++;                                  	\
	if ((pxHandle)->ucCurrentActiveDigit == (pxHandle)->ucNumberOfDigits)  	\
		(pxHandle)->ucCurrentActiveDigit = 0;                           	\
}

/*
 * Gets the DIO level that achieves the requested "ucState" based on "ucActiveLevel".
 *
 * Truth table:
 *       ucState   ucActiveLevel   level
 *         0             0           1
 *         0             1           0
 *         1             0           0
 *         1             1           1
 */
#define ucGET_LEVEL(ucState, ucActiveLevel) (!((ucState) ^ (ucActiveLevel)))

static inline void vWriteCurrentlyActiveDigit(xHOS_SevenSegmentMux_t* pxHandle)
{
	/*	Extract values	*/
	uint8_t* pxSegPortNumberArr = pxHandle->pxSegmentPortNumberArr;
	uint8_t* pxSegPinNumberArr = pxHandle->pxSegmentPinNumberArr;
	uint8_t ucActiveLevel = pxHandle->ucSegmentActiveLevel;
	uint8_t ucCurrent = pxHandle->ucCurrentActiveDigit;
	uint8_t ucVal = pxHandle->pucDisplayBuffer[ucCurrent];
	uint8_t ucStates = pucSegmentStateArr[ucVal];
	uint8_t ucPort, ucPin, ucState, ucLevel;
	uint8_t i;

	/*	Apply states on the segments	*/
	for (i = 0; i < 7; i++)
	{
		ucPort = pxSegPortNumberArr[i];
		ucPin = pxSegPinNumberArr[i];
		ucState = ucGET_BIT(ucStates, i);
		ucLevel = ucGET_LEVEL(ucState, ucActiveLevel);
		vPORT_DIO_WRITE_PIN(ucPort, ucPin, ucLevel);
	}

	/*	if point active is on the currently active digit, activate it, otherwise de-activate it	*/
	ucState = pxHandle->ucPointIndex & (1 << ucCurrent);
//	ucState = (pxHandle->cPointIndex == (int8_t)ucCurrent);
	ucPort = pxSegPortNumberArr[7];
	ucPin = pxSegPinNumberArr[7];
	ucLevel = ucGET_LEVEL(ucState, ucActiveLevel);
	vPORT_DIO_WRITE_PIN(ucPort, ucPin, ucLevel);
}

/*******************************************************************************
 * RTOS Task code:
 ******************************************************************************/
static void vTask(void* pvParams)
{
	xHOS_SevenSegmentMux_t* pxHandle = (xHOS_SevenSegmentMux_t*)pvParams;

	TickType_t xLastWakeTime = xTaskGetTickCount();
	while(1)
	{
		/*	disable currently active digit	*/
		vDisableCurrentlyActiveDigit(pxHandle);

		/*	increment "currentActiveDigit" counter	*/
		vINCREMENT_CURRENT_ACTIVE_COUNTER(pxHandle);

		/*	write currently active digit from the buffer	*/
		vWriteCurrentlyActiveDigit(pxHandle);

		/*	enable currently active digit	*/
		vEnableCurrentlyActiveDigit(pxHandle);

		/*	Task is blocked until next update time	*/
		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(pxHandle->uiUpdatePeriodMs));
	}
}

/*******************************************************************************
 * API functions:
 ******************************************************************************/
/*
 * See header file for info.
 */
void vHOS_SevenSegmentMux_init(xHOS_SevenSegmentMux_t* pxHandle)
{
	uint8_t i;
	uint8_t ucPort, ucPin;
	uint8_t ucLevel;
	static uint8_t ucCreatedObjectsCount = 0;
	char pcTaskName[configMAX_TASK_NAME_LEN];

	pxHandle->ucIsEnabled = 0;

	/*	initialize segments pins as digital outputs, initially in-active	*/
	ucLevel = ucGET_LEVEL(0, pxHandle->ucSegmentActiveLevel);
	for (i = 0; i < 8; i++)
	{
		ucPort = pxHandle->pxSegmentPortNumberArr[i];
		ucPin = pxHandle->pxSegmentPinNumberArr[i];
		vPort_DIO_initPinOutput(ucPort, ucPin);
		vPORT_DIO_WRITE_PIN(ucPort, ucPin, ucLevel);
	}

	/*	initialize digit enable pins as digital outputs, initially in-active	*/
	ucLevel = ucGET_LEVEL(0, pxHandle->ucEnableActiveLevel);
	for (i = 0; i < pxHandle->ucNumberOfDigits; i++)
	{
		ucPort = pxHandle->pxDigitEnablePortNumberArr[i];
		ucPin = pxHandle->pxDigitEnablePinNumberArr[i];
		vPort_DIO_initPinOutput(ucPort, ucPin);
		vPORT_DIO_WRITE_PIN(ucPort, ucPin, ucLevel);
	}

	/*	display buffer is initially all-zeros	*/
	for (i = 0; i < pxHandle->ucNumberOfDigits; i++)
	{
		pxHandle->pucDisplayBuffer[i] = 0;
	}

	/*	Initially, point is disabled	*/
	pxHandle->ucPointIndex = 0;

	/*	Create task	*/
	sprintf(pcTaskName, "SSMux%d", ucCreatedObjectsCount++);

	pxHandle->xTask = xTaskCreateStatic(	vTask,
											pcTaskName,
											uiHOS_SEVENSEGMENTMUX_STACK_SZ,
											(void*)pxHandle,
											configHOS_SOFT_REAL_TIME_TASK_PRI,
											pxHandle->puxTaskStack,
											&pxHandle->xTaskStatic	);
}

/*
 * See header file for info.
 */
void vHOS_SevenSegmentMux_write(	xHOS_SevenSegmentMux_t* pxHandle,
									uint32_t uiNum,
									uint8_t ucPointIndex	)
{
    uint8_t i;
    uint8_t ucDigVal;
    uint8_t* pucDigArr = pxHandle->pucDisplayBuffer;
    uint8_t ucNDigs = pxHandle->ucNumberOfDigits;

    for (i = 0; i < ucNDigs; i++)
    {
    	ucDigVal = uiNum % 10;
    	pucDigArr[i] = ucDigVal;
    	uiNum = uiNum / 10;
        if (uiNum == 0)
            break;
    }
    for (i = i + 1; i < ucNDigs; i++)
    {
    	pucDigArr[i] = 0;
    }

    pxHandle->ucPointIndex = ucPointIndex;
}

/*
 * See header file for info.
 */
__attribute__((always_inline)) inline
void vHOS_SevenSegmentMux_Enable(xHOS_SevenSegmentMux_t* pxHandle)
{
	pxHandle->ucIsEnabled = 1;

	vTaskResume(pxHandle->xTask);
}

/*
 * See header file for info.
 */
void vHOS_SevenSegmentMux_Disable(xHOS_SevenSegmentMux_t* pxHandle)
{
	vTaskSuspend(pxHandle->xTask);

	pxHandle->ucIsEnabled = 0;

	/*	disable all digits	*/
	for (uint8_t i = 0; i < pxHandle->ucNumberOfDigits; i++)
	{
		vPORT_DIO_WRITE_PIN(	pxHandle->pxDigitEnablePortNumberArr[i],
							pxHandle->pxDigitEnablePinNumberArr[i],
							!(pxHandle->ucEnableActiveLevel));
	}
}

