/*
 * BinaryFilter.c
 *
 *  Created on: Aug 5, 2023
 *      Author: Ali Emad
 */

/*	LIB	*/
#include "stdint.h"

/*	SELF	*/
#include "LIB/BinaryFilter/BinaryFilter.h"

/*
 * See header file for info.
 */
void vLIB_BinaryFilter_init(xLIB_BinaryFilter_t* pxHandle)
{
	pxHandle->ucPrevLevelFiltered = 0;
	pxHandle->ucLevelFiltered = 0;
	pxHandle->ucPrevLevel = 0;
	pxHandle->ucCount = 0;
}

/*
 * See header file for info.
 */
void vLIB_BinaryFilter_updateFilter(xLIB_BinaryFilter_t* pxHandle, uint8_t ucNewLevel)
{
	/*	Update "prevFiltered" value	*/
	pxHandle->ucPrevLevelFiltered = pxHandle->ucLevelFiltered;

	/*	If new level is same as previous level (no noise occurred)	*/
	if (ucNewLevel == pxHandle->ucPrevLevel)
	{
		pxHandle->ucCount++;
		/*	If N samples on the same level were measured	*/
		if (pxHandle->ucCount == pxHandle->ucNFilter)
		{
			/*	Filter's level is now stable and could be used	*/
			pxHandle->ucLevelFiltered = ucNewLevel;
			pxHandle->ucCount = 0;
		}
	}

	/*	Otherwise, (noise occurred)	*/
	else
	{
		pxHandle->ucCount = 0;
		pxHandle->ucPrevLevel = ucNewLevel;
	}
}
