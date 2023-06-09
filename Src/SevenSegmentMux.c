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
#include "Port/Port_DIO.h"
#include "Port/Port_Print.h"
#include "Port/Port_Breakpoint.h"

/*	SELF	*/
#include "Inc/SevenSegmentMux/SevenSegmentMux.h"

#if configHOS_SEVEN_SEGMENT_EN

static xHOS_SevenSegmentMux_t pxSevenSegmentMuxArr[configHOS_SEVEN_SEGMENT_MUX_MAX_NUMBER_OF_DIGITS];
static uint16_t usNumberOfUsedHandles = 0;

static uint8_t pucSegmentStateArr[16] = {

};

/*
 * Helping functions/macros.
 */
static inline vDisableCurrentlyActiveDigit(xHOS_SevenSegmentMux_t* pxHandle)
{
	uint8_t current = pxHandle->ucCurrentActiveDigit;
	vHOS_DIO_writePin(	pxHandle->pxDigitEnablePortPinArr[current].unPort,
						pxHandle->pxDigitEnablePortPinArr[current].unPin,
						!(pxHandle->ucEnableActiveLevel)	);
}

static inline vEnableCurrentlyActiveDigit(xHOS_SevenSegmentMux_t* pxHandle)
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

/*	TODO: test this	*/
#define ucNIBBLE(pucArr, i) (((pucArr)[(i)/2] >> (((i)%2)*4)) & 0x0F)

static inline vWriteCurrentlyActiveDigit(xHOS_SevenSegmentMux_t* pxHandle)
{

}

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


		}

		/*	Delay until next update time	*/
		vTaskDelay(pdMS_TO_TICKS(configHOS_SEVEN_SEGMENT_MUX_UPDATE_TIME_MS));
	}
}

/*
 * See header file for info.
 */
xHOS_SevenSegmentMux_t* pxHOS_SevenSegmentMux_init(	xHOS_DIO_t* pxSegmentPortPinArr,
													xHOS_DIO_t* pxDigitEnablePortPinArr,
													uint8_t ucSegmentActiveLevel,
													uint8_t ucEnableActiveLevel,
													uint8_t ucNumberOfDigits	);

/*
 * See header file for info.
 */
void vHOS_SevenSegmentMux_write(	xHOS_SevenSegmentMux_t* pxSevenSegmentMuxHandle,
									uint32_t uiNum,
									int8_t cPointIndex	);

/*
 * See header file for info.
 */
void vHOS_SevenSegmentMux_Enable(xHOS_SevenSegmentMux_t* pxSevenSegmentMuxHandle);

/*
 * See header file for info.
 */
void vHOS_SevenSegmentMux_Disable(xHOS_SevenSegmentMux_t* pxSevenSegmentMuxHandle);


#endif	/*	configHOS_SEVEN_SEGMENT_EN	*/
