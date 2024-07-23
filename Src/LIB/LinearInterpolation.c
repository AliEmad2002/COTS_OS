/*
 * LinearInterpolation.c
 *
 *  Created on: Jul 21, 2024
 *      Author: Ali Emad
 */

#include <stdint.h>

#include "LIB/LinearInterpolation/LinearInterpolation.h"

#define GET_Y(X1, X2, Y1, Y2, X)	\
	((((X)-(X1)) * ((Y1)-(Y2))) / ((X1)-(X2)) + Y1)

int32_t iLIB_LinearInterpolation_interpolate(
		int32_t* piXArr,
		int32_t* piYArr,
		uint32_t uiLen,
		int32_t iX
	)
{
	int32_t iY;
	int32_t iX1, iX2, iY1, iY2;

	/*	If "iX" is less than 1st value of "piXArr[]"	*/
	if (iX < piXArr[0])
	{
		iX1 = piXArr[0];
		iX2 = piXArr[1];
		iY1 = piYArr[0];
		iY2 = piYArr[1];
	}

	/*	If "iX" is larger than last value of "piXArr[]"	*/
	else if (iX >= piXArr[uiLen-1])
	{
		iX1 = piXArr[uiLen-2];
		iX2 = piXArr[uiLen-1];
		iY1 = piYArr[uiLen-2];
		iY2 = piYArr[uiLen-1];
	}

	/*	Otherwise, if "iX" is within the range of "piXArr[]"	*/
	else
	{
		for (uint32_t i = 0; i < uiLen; i++)
		{
			if (iX >= piXArr[i])
			{
				iX1 = piXArr[i];
				iX2 = piXArr[i+1];
				iY1 = piYArr[i];
				iY2 = piYArr[i+1];
			}
		}
	}

	iY = GET_Y(iX1, iX2, iY1, iY2, iX);

	return iY;
}
