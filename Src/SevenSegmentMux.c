/*
 * SevenSegmentMux.c
 *
 *  Created on: Jun 9, 2023
 *      Author: Ali Emad
 */

/*	LIB	*/
#include <stdint.h>
#include "Library/Bit_Math.h"

/*	FreeRTOS	*/
#include "FreeRTOS.h"
#include "task.h"

/*	MCAL (Ported)	*/
#include "Port/Port_DIO.h"
#include "Port/Port_Print.h"
#include "Port/Port_Breakpoint.h"

/*	SELF	*/
#include "Inc/SevenSegmentMux/SevenSegmentMux.h"

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
	uint8_t current = pxHandle->ucCurrentActiveDigit;
	vHOS_DIO_writePin(	pxHandle->pxDigitEnablePortPinArr[current].unPort,
						pxHandle->pxDigitEnablePortPinArr[current].unPin,
						!(pxHandle->ucEnableActiveLevel)	);
}

static inline void vEnableCurrentlyActiveDigit(xHOS_SevenSegmentMux_t* pxHandle)
{
	uint8_t current = pxHandle->ucCurrentActiveDigit;
	vHOS_DIO_writePin(	pxHandle->pxDigitEnablePortPinArr[current].unPort,
						pxHandle->pxDigitEnablePortPinArr[current].unPin,
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
	xHOS_DIO_t* pxSegArr = pxHandle->pxSegmentPortPinArr;
	uint8_t ucActiveLevel = pxHandle->ucSegmentActiveLevel;
	uint8_t ucCurrent = pxHandle->ucCurrentActiveDigit;
	uint8_t ucVal = ucGET_NIBBLE(pxHandle->pucDisplayBuffer, ucCurrent);
	uint8_t ucStates = pucSegmentStateArr[ucVal];
	uint8_t ucPort, ucPin, ucState, ucLevel;
	uint8_t i;

	/*	Apply states on the segments	*/
	for (i = 0; i < 7; i++)
	{
		ucPort = pxSegArr[i].unPort;
		ucPin = pxSegArr[i].unPin;
		ucState = ucGET_BIT(ucStates, i);
		ucLevel = ucGET_LEVEL(ucState, ucActiveLevel);
		vHOS_DIO_writePin(ucPort, ucPin, ucLevel);
	}

	/*	if point active is on the currently active digit, activate it, otherwise de-activate it	*/
	ucState = (pxHandle->cPointIndex == (int8_t)ucCurrent);
	ucPort = pxSegArr[7].unPort;
	ucPin = pxSegArr[7].unPin;
	ucLevel = ucGET_LEVEL(ucState, ucActiveLevel);
	vHOS_DIO_writePin(ucPort, ucPin, ucLevel);
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
													configNORMAL_TASK_PRIORITY,
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
xHOS_SevenSegmentMux_t* pxHOS_SevenSegmentMux_init(	xHOS_DIO_t* pxSegmentPortPinArr,
													xHOS_DIO_t* pxDigitEnablePortPinArr,
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
		ucPort = pxSegmentPortPinArr[i].unPort;
		ucPin = pxSegmentPortPinArr[i].unPin;
		vHOS_DIO_initPinOutput(ucPort, ucPin);
		vHOS_DIO_writePin(ucPort, ucPin, ucGET_LEVEL(0, ucSegmentActiveLevel));
	}

	/*	initialize digit enable pins as digital outputs, initially in-active	*/
	for (i = 0; i < ucNumberOfDigits; i++)
	{
		ucPort = pxDigitEnablePortPinArr[i].unPort;
		ucPin = pxDigitEnablePortPinArr[i].unPin;
		vHOS_DIO_initPinOutput(ucPort, ucPin);
		vHOS_DIO_writePin(ucPort, ucPin, ucGET_LEVEL(0, ucEnableActiveLevel));
	}

	/*	add new object to driver's objects static array	*/
	pxHandle = &pxSevenSegmentMuxArr[usNumberOfUsedHandles];
	pxHandle->pxSegmentPortPinArr = pxSegmentPortPinArr;
	pxHandle->pxDigitEnablePortPinArr = pxDigitEnablePortPinArr;
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

	pxSevenSegmentMuxHandle->ucIsEnabled = 0;
}


#endif	/*	configHOS_SEVEN_SEGMENT_EN	*/
