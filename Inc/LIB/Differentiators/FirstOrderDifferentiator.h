/*
 * FirstOrderDifferentiator.h
 *
 *  Created on: Aug 9, 2023
 *      Author: Ali Emad
 *
 * Using Z-transform, and after a mathematical derivation:
 * (dx/dt)[n] = 2/T * x[n] - 2/T * x[n-1] - (dx/dt)[n-1]
 */

#ifndef COTS_OS_INC_LIB_DIFFERENTIATORS_FIRSTORDERDIFFERENTIATOR_H_
#define COTS_OS_INC_LIB_DIFFERENTIATORS_FIRSTORDERDIFFERENTIATOR_H_


typedef struct{
	/*		PRIVATE		*/
	int32_t iXPrev;

	/*		PUBLIC		*/
	int32_t iDiff;	// Read-only.
	uint32_t uiDeltaTimeMs;
}xLIB_FirstOrderDifferentiator_t;

/*
 * Initializes handle.
 *
 * "uiDeltaTimeMs" must be previously set.
 */
void vLIB_FirstOrderDifferentiator_init(xLIB_FirstOrderDifferentiator_t* pxHandle);

/*
 * Updates handle with new value.
 *
 * 		-	This function MUST be frequently called in a period that is as close
 * 			as possible to that defined in "uiDeltaTimeMs", in order to obtain
 * 			realistic derivative values.
 */
void vLIB_FirstOrderDifferentiator_update(	xLIB_FirstOrderDifferentiator_t* pxHandle,
											int32_t ixNew	);

#endif /* COTS_OS_INC_LIB_DIFFERENTIATORS_FIRSTORDERDIFFERENTIATOR_H_ */
