/*
 * SevenSegmentMux.c
 *
 *  Created on: Jun 9, 2023
 *      Author: Ali Emad
 */

/*	LIB	*/
#include <stdint.h>

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

#if configHOS_SEVEN_SEGMENT_EN

/*******************************************************************************
 * Private configurations:
 ******************************************************************************/
#define uiSEVEN_SEGMENT_MUX_STACK_SIZE	configMINIMAL_STACK_SIZE

/*******************************************************************************
 * Static objects:
 ******************************************************************************/
static xHOS_SevenSegmentMux_t pxSevenSegmentMuxArr[configHOS_SEVEN_SEGMENT_MUX_MAX_NUMBER_OF_DIGITS];
static uint16_t usNumberOfUsedHandles = 0;

static uint8_t pucSegmentStateArr[] = {
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

/*	Driver's stacks and task handle	*/
static StackType_t pxSevenSegmentMuxStack[uiSEVEN_SEGMENT_MUX_STACK_SIZE];
static StaticTask_t xSevenSegmentMuxStaticTask;
static TaskHandle_t xSevenSegmentMuxTaskHandle;

/*******************************************************************************
 * Helping functions/macros.
 ******************************************************************************/
static inline void vDisableCurrentlyActiveDigit(xHOS_SevenSegmentMux_t* pxHandle)
{
	uint8_t ucCurrent = pxHandle->ucCurrentActiveDigit;
	vPort_DIO_writePin(	pxHandle->pxDigitEnablePortNumberArr[ucCurrent],
						pxHandle->pxDigitEnablePinNumberArr[ucCurrent],
						!(pxHandle->ucEnableActiveLevel)	);
}

static inline void vEnableCurrentlyActiveDigit(xHOS_SevenSegmentMux_t* pxHandle)
{
	uint8_t ucCurrent = pxHandle->ucCurrentActiveDigit;
	vPort_DIO_writePin(	pxHandle->pxDigitEnablePortNumberArr[ucCurrent],
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
	uint8_t ucVal = ucGET_NIBBLE(pxHandle->pucDisplayBuffer, ucCurrent);
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
		vPort_DIO_writePin(ucPort, ucPin, ucLevel);
	}

	/*	if point active is on the currently active digit, activate it, otherwise de-activate it	*/
	ucState = (pxHandle->cPointIndex == (int8_t)ucCurrent);
	ucPort = pxSegPortNumberArr[7];
	ucPin = pxSegPinNumberArr[7];
	ucLevel = ucGET_LEVEL(ucState, ucActiveLevel);
	vPort_DIO_writePin(ucPort, ucPin, ucLevel);
}

#define pxTOP_PTR	\
	(	pxSevenSegmentMuxArr + \
		configHOS_SEVEN_SEGMENT_MUX_MAX_NUMBER_OF_OBJECTS * sizeof(xHOS_SevenSegmentMux_t)	)

/*******************************************************************************
 * RTOS Task code:
 ******************************************************************************/
/*
 * If "configHOS_SEVEN_SEGMENT_EN" was enabled, a task that uses this function will be
 * created in the "xHOS_init()".
 *
 * For further information about how HAL_OS drivers are managed, refer to the
 * repository's "README.md"
 */
void vSevenSegmentMux_manager(void* pvParams)
{
	uint8_t i;
	xHOS_SevenSegmentMux_t* pxHandle;

	while(1)
	{
		/*	loop on created handles	*/
		for (i = 0; i < usNumberOfUsedHandles; i++)
		{
			pxHandle = &pxSevenSegmentMuxArr[i];

			/*	if handle is disabled, skip it	*/
			if (pxHandle->ucIsEnabled == 0)
				continue;

			/*	disable currently active digit	*/
			vDisableCurrentlyActiveDigit(pxHandle);

			/*	increment "currentActiveDigit" counter	*/
			vINCREMENT_CURRENT_ACTIVE_COUNTER(pxHandle);

			/*	write currently active digit from the buffer	*/
			vWriteCurrentlyActiveDigit(pxHandle);

			/*	enable currently active digit	*/
			vEnableCurrentlyActiveDigit(pxHandle);
		}

		/*	Delay until next update time	*/
		vTaskDelay(pdMS_TO_TICKS(configHOS_SEVEN_SEGMENT_MUX_UPDATE_TIME_MS));
	}
}

/*
 * Initializes the task responsible for this  driver.
 * Notes:
 * 		-	This function is externed and called in "HAL_OS.c".
 */
BaseType_t xSevenSegmentMux_initTask(void)
{
	xSevenSegmentMuxTaskHandle = xTaskCreateStatic(	vSevenSegmentMux_manager,
													"SSMux",
													uiSEVEN_SEGMENT_MUX_STACK_SIZE,
													NULL,
													configHOS_SOFT_REAL_TIME_TASK_PRI,
													pxSevenSegmentMuxStack,
													&xSevenSegmentMuxStaticTask	);
	if (xSevenSegmentMuxTaskHandle == NULL)
		return pdFAIL;

	else
		return pdPASS;
}

/*******************************************************************************
 * API functions:
 ******************************************************************************/
/*
 * See header file for info.
 */
xHOS_SevenSegmentMux_t* pxHOS_SevenSegmentMux_init(	uint8_t* pxSegmentPortNumberArr,
													uint8_t* pxSegmentPinNumberArr,
													uint8_t* pxDigitEnablePortNumberArr,
													uint8_t* pxDigitEnablePinNumberArr,
													uint8_t ucSegmentActiveLevel,
													uint8_t ucEnableActiveLevel,
													uint8_t ucNumberOfDigits	)
{
	uint8_t i;
	uint8_t ucPort, ucPin;
	xHOS_SevenSegmentMux_t* pxHandle;

	/*	check (assert) if there's enough memory to add new button	*/
	configASSERT(usNumberOfUsedHandles < configHOS_SEVEN_SEGMENT_MUX_MAX_NUMBER_OF_OBJECTS);

	/*	initialize segments pins as digital outputs, initially in-active	*/
	for (i = 0; i < 8; i++)
	{
		ucPort = pxSegmentPortNumberArr[i];
		ucPin = pxSegmentPinNumberArr[i];
		vPort_DIO_initPinOutput(ucPort, ucPin);
		vPort_DIO_writePin(ucPort, ucPin, ucGET_LEVEL(0, ucSegmentActiveLevel));
	}

	/*	initialize digit enable pins as digital outputs, initially in-active	*/
	for (i = 0; i < ucNumberOfDigits; i++)
	{
		ucPort = pxDigitEnablePortNumberArr[i];
		ucPin = pxDigitEnablePinNumberArr[i];
		vPort_DIO_initPinOutput(ucPort, ucPin);
		vPort_DIO_writePin(ucPort, ucPin, ucGET_LEVEL(0, ucEnableActiveLevel));
	}

	/*	add new object to driver's objects static array	*/
	pxHandle = &pxSevenSegmentMuxArr[usNumberOfUsedHandles];
	pxHandle->pxSegmentPortNumberArr = pxSegmentPortNumberArr;
	pxHandle->pxSegmentPinNumberArr = pxSegmentPinNumberArr;
	pxHandle->pxDigitEnablePortNumberArr = pxDigitEnablePortNumberArr;
	pxHandle->pxDigitEnablePinNumberArr = pxDigitEnablePinNumberArr;
	pxHandle->ucSegmentActiveLevel = ucSegmentActiveLevel;
	pxHandle->ucEnableActiveLevel = ucEnableActiveLevel;
	pxHandle->ucIsEnabled = 0;
	pxHandle->ucNumberOfDigits = ucNumberOfDigits;
	pxHandle->ucCurrentActiveDigit = 0;
	pxHandle->cPointIndex = -1;

	/*	display buffer is initially all-zeros	*/
	for (i = 0; i < (ucNumberOfDigits+1)/2; i++)
	{
		pxHandle->pucDisplayBuffer[i] = 0;
	}

	/*	increment created handles counter	*/
	usNumberOfUsedHandles++;

	return pxHandle;
}

/*
 * See header file for info.
 */
void vHOS_SevenSegmentMux_write(	xHOS_SevenSegmentMux_t* pxSevenSegmentMuxHandle,
									uint32_t uiNum,
									int8_t cPointIndex	)
{
	/*	check pointer first	*/
	configASSERT(
		(pxSevenSegmentMuxArr <= pxSevenSegmentMuxHandle) &&
		(pxSevenSegmentMuxHandle < pxTOP_PTR));

    uint8_t i;
    uint8_t ucDigVal;
    uint8_t* pucDigArr = pxSevenSegmentMuxHandle->pucDisplayBuffer;
    uint8_t ucNDigs = pxSevenSegmentMuxHandle->ucNumberOfDigits;

    for (i = 0; i < ucNDigs; i++)
    {
    	ucDigVal = uiNum % 10;
    	vWRT_NIBBLE(pucDigArr, i, ucDigVal);
    	uiNum = uiNum / 10;
        if (uiNum == 0)
            break;
    }
    for (i = i + 1; i < ucNDigs; i++)
    {
    	vWRT_NIBBLE(pucDigArr, i, 0);
    }

    pxSevenSegmentMuxHandle->cPointIndex = cPointIndex;
}

/*
 * See header file for info.
 */
void vHOS_SevenSegmentMux_Enable(xHOS_SevenSegmentMux_t* pxSevenSegmentMuxHandle)
{
	/*	check pointer first	*/
	configASSERT(
		(pxSevenSegmentMuxArr <= pxSevenSegmentMuxHandle) &&
		(pxSevenSegmentMuxHandle < pxTOP_PTR));

	pxSevenSegmentMuxHandle->ucIsEnabled = 1;
}

/*
 * See header file for info.
 */
void vHOS_SevenSegmentMux_Disable(xHOS_SevenSegmentMux_t* pxSevenSegmentMuxHandle)
{
	/*	check pointer first	*/
	configASSERT(
		(pxSevenSegmentMuxArr <= pxSevenSegmentMuxHandle) &&
		(pxSevenSegmentMuxHandle < pxTOP_PTR));

	/*	disable all segments	*/
	for (uint8_t i = 0; i < pxSevenSegmentMuxHandle->ucNumberOfDigits; i++)
	{
		vPort_DIO_writePin(	pxSevenSegmentMuxHandle->pxDigitEnablePortNumberArr[i],
							pxSevenSegmentMuxHandle->pxDigitEnablePinNumberArr[i],
							!(pxSevenSegmentMuxHandle->ucEnableActiveLevel));
	}

	pxSevenSegmentMuxHandle->ucIsEnabled = 0;
}


#endif	/*	configHOS_SEVEN_SEGMENT_EN	*/
