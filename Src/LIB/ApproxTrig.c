/*
 * ApproxTrig.c
 *
 *  Created on: Sep 28, 2023
 *      Author: Ali Emad
 */

#include "stdint.h"
#include "stdlib.h"

#include "LIB/ApproxTrig.h"

/*
 * See header for info.
 */
int32_t iLIB_ApproxTrig_Sin1(uint64_t ulX)
{
	int32_t x = ulX % 1000;

	if (x < 500)
		return (- ((x * x) << 1) + 1000 * x) / 125;
	else
		return (((x * x) << 1) - 3000 * x + 1000000) / 125;
}

/*
 * See header for info.
 */
int64_t lLIB_ApproxTrig_Sin2(uint64_t ulX)
{
	int64_t x = ulX % 1000000;

	if (x < 500000)
		return (-x * x + 500000 * x) / 62500;
	else
		return (x * x - 1500000 * x + 500000000000) / 62500;
}

/*
 * See header for info.
 */
float fLIB_ApproxTrig_Atan1(float fX)
{
	float fAbsX = abs(fX);
	return 0.785398163f * fX - fX * (fAbsX - 1.0f) * (0.2447f + 0.0663f * fAbsX);
}

/*
 * See header for info.
 */
int32_t iLIB_ApproxTrig_Atan2(int32_t iX)
{
	int64_t lX = iX;
	int64_t lAbsX = abs(iX);
	return (785398163 * lX - lX * (lAbsX - 1000) * (244700 + 66 * lAbsX)) / 1000000000;
}
