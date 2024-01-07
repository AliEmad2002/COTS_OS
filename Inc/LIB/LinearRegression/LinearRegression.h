/*
 * LinearRegression.h
 *
 *  Created on: Dec 29, 2023
 *      Author: Ali Emad
 *
 * This driver uses least squares method.
 * Recap on theory: https://youtu.be/P8hT5nDai6A?si=yd-YLf4gRHTHIrSz
 */

#ifndef COTS_OS_INC_LIB_LINEARREGRESSION_LINEARREGRESSION_H_
#define COTS_OS_INC_LIB_LINEARREGRESSION_LINEARREGRESSION_H_

typedef struct{
	/*		PRIVATE		*/
	float lXYSum;
	float lX2Sum;
	float ulN;
	float iXSum;
	float iYSum;

	/*		PUBLIC (Read only)		*/
	/*
	 * Line equation parameters, such that:
	 * 		-	Y = A * X + B
	 */
	float iA;
	float iB;
}xLIB_LinearRegression_t;

/*
 * Initializes handle
 */
void vLIB_LinearRegression_init(xLIB_LinearRegression_t* pxHandle);

/*
 * Updates handle with new sample.
 */
void vLIB_LinearRegression_update(	xLIB_LinearRegression_t* pxHandle,
		float iX,
		float iY	);




#endif /* COTS_OS_INC_LIB_LINEARREGRESSION_LINEARREGRESSION_H_ */
