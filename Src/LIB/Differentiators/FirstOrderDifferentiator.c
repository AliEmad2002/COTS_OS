/*
 * FirstOrderDifferentiator.c
 *
 *  Created on: Aug 9, 2023
 *      Author: Ali Emad
 */

/*	LIB	*/
#include "stdint.h"

/*	SELF	*/
#include "LIB/Differentiators/FirstOrderDifferentiator.h"

/*
 * See header for info.
 */
void vLIB_FirstOrderDifferentiator_init(xLIB_FirstOrderDifferentiator_t* pxHandle)
{
	pxHandle->iXPrev = 0;
	pxHandle->iDiff = 0;
}

/*
 * See header for info.
 */
void vLIB_FirstOrderDifferentiator_update(	xLIB_FirstOrderDifferentiator_t* pxHandle,
											int32_t ixNew	)
{
	pxHandle->iDiff =
		(2000 * (ixNew - pxHandle->iXPrev)) /
		pxHandle->uiDeltaTimeMs - pxHandle->iDiff;

	pxHandle->iXPrev = ixNew;
}
