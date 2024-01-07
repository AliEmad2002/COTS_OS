/*
 * LinearRegression.c
 *
 *  Created on: Dec 29, 2023
 *      Author: Ali Emad
 */



/*	LIB	*/
#include "stdint.h"

/*	SELF	*/
#include "LIB/LinearRegression/LinearRegression.h"


/*
 * See header for info.
 */
void vLIB_LinearRegression_init(xLIB_LinearRegression_t* pxHandle)
{
	pxHandle->lXYSum = 0;
	pxHandle->iXSum = 0;
	pxHandle->iYSum = 0;
	pxHandle->lX2Sum = 0;
	pxHandle->ulN = 0;
	pxHandle->iA = 0;
	pxHandle->iB = 0;
}

/*
 * See header for info.
 */
void vLIB_LinearRegression_update(	xLIB_LinearRegression_t* pxHandle,
		float iX,
		float iY	)
{
	pxHandle->lXYSum += iX * iY;
	pxHandle->iXSum += iX;
	pxHandle->iYSum += iY;
	pxHandle->lX2Sum += iX * iX;
	pxHandle->ulN++;

	pxHandle->iA =
			( pxHandle->lXYSum - pxHandle->iXSum * pxHandle->iYSum ) /
			( pxHandle->ulN * pxHandle->lX2Sum - pxHandle->iXSum * pxHandle->iXSum )	;

	pxHandle->iB =
			(pxHandle->iYSum - pxHandle->iA * pxHandle->iXSum ) /
			pxHandle->ulN	;
}
