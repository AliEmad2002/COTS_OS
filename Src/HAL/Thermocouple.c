/*
 * Thermocouple.c
 *
 *  Created on: Dec 31, 2023
 *      Author: Ali Emad
 */

/*	LIB	*/
#include "stdint.h"
#include "LIB/Assert.h"

/*	RTOS	*/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "RTOS_PRI_Config.h"

/*	MCAL	*/

/*	HAL	*/
#include "HAL/CalibratableAmplifier/CalibratableAmplifier.h"
#include "HAL/AnalogLinearTemperatureSensor/AnalogLinearTemperatureSensor.h"

/*	SELF	*/
#include "HAL/Thermocouple/Thermocouple_Table.h"
#include "HAL/Thermocouple/Thermocouple.h"

/*******************************************************************************
 * Helping macros:
 ******************************************************************************/
#define iINTERPOLATE(iX1, iX2, iY1, iY2, iX)	\
	(	(iY1) + (((iY2) - (iY1)) * ((iX) - (iX1))) / ((iX1) - (iX2))	)

/*******************************************************************************
 * Static (Private) functions:
 ******************************************************************************/
/*
 * Returns voltage in micro-volts, using given temperature value to interpolate
 * in the data table.
 *
 * Notes:
 * 		-	"iT": is temperature in milli-C.
 *
 * 		-	Result is stored in "piV".
 *
 * 		-	Function returns 0 if the given temperature is out of the range of
 * 			data tale. Otherwise it returns 1.
 */
static uint8_t ucInterpolateUsingTemperature(int32_t iT, int32_t* piV)
{
	/*	Get nearest lower temperature (in C)	 */
	int32_t iTL = iT / 1000;

	/*	Range check	*/
	if (	(0 > iTL)	||	(iTL > MAX_TEMP)	)
		return 0;

	if ( iTL == MAX_TEMP) // No interpolation needed
	{
		*piV = pusArr[MAX_TEMP];
		return 1;
	}

	/*	Get voltage of nearest lower and higher temperatures	*/
	int32_t iVL = pusArr[iTL];
	int32_t iVH = pusArr[iTL + 1];

	/*	Interpolate	*/
	iTL = iTL * 1000;
	int32_t iTH = iTL + 1000;

	*piV = iINTERPOLATE(iTL, iTH, iVL, iVH, iT);

	return 1;
}

/*
 * Returns temperature in milli-C, using given voltage value to interpolate
 * in the data table.
 *
 * Notes:
 * 		-	"iV": is temperature in micro-volts.
 *
 * 		-	Result is stored in "piT".
 *
 * 		-	Function returns 0 if the given voltage is out of the range of
 * 			data tale. Otherwise it returns 1.
 */
static uint8_t ucInterpolateUsingVoltage(	xHOS_Thermocouple_t* pxHandle,
											int32_t iV,
											int32_t* piT	)
{
	/*	Range check	*/
	if (	(pusArr[0] > iV)	||	(iV > pusArr[MAX_TEMP])	)
		return 0;

	if (iV == pusArr[MAX_TEMP]) // No interpolation needed.
	{
		*piT = MAX_TEMP * 1000;
		return 1;
	}

	/*
	 * Get nearest lower and higher temperatures (Use last reading as a search
	 * starting point).
	 */
	/*	Calculate incrementer (Search direction)	*/
	int32_t iInc = (pusArr[pxHandle->uiTLPrev] < iV) ? 1 : -1;

	/*	While "iV" is not in between "data[tPrev]" and "data[tPrev+1]"	*/
	while (!(pusArr[pxHandle->uiTLPrev] <= iV && iV < pusArr[pxHandle->uiTLPrev+1]))
	{
		/*	increment	*/
		pxHandle->uiTLPrev += iInc;
	}

	/*	Interpolate	*/
	int32_t iTL = pxHandle->uiTLPrev;
	int32_t iVL = pusArr[iTL];
	int32_t iVH = pusArr[iTL + 1];
	iTL = iTL * 1000;
	int32_t iTH = iTL + 1000;

	*piT = iINTERPOLATE(iVL, iVH, iTL, iTH, iV);

	return 1;
}


/*******************************************************************************
 * API functions:
 ******************************************************************************/
/*
 * See header for info.
 */
void vHOS_Thermocouple_init(xHOS_Thermocouple_t* pxHandle)
{
	pxHandle->uiTLPrev = 0;
}

uint8_t ucHOS_Thermocouple_getTemperature(	xHOS_Thermocouple_t* pxHandle,
											int32_t* piT	)
{
	/*	Get ambient temperature	*/
	int32_t iTA = iHOS_AnalogLinearTemperatureSensor_getTemperature(
			pxHandle->pxColdJunctionTemperatureSensor	);

	/*
	 * Get voltage of reference junction (voltage corresponding to ambient
	 * temperature).
	 */
	int32_t iVColdJunction;
	if (!ucInterpolateUsingTemperature(iTA, &iVColdJunction))
		return 0;

	/*	Get voltage induced by the thermocouple	*/
	int32_t iVThermocouple = iHOS_CalibratableAmplifier_read(
			pxHandle->pxAmplifier, portMAX_DELAY	);

	/*	Get total thermocouple voltage due to zero temperature reference	*/
	int32_t iV = iVColdJunction + iVThermocouple;

	/*	Get temperature corresponding to this voltage	*/
	if (!ucInterpolateUsingVoltage(pxHandle, iV, piT))
		return 0;

	return 1;
}







