/*
 * LinearInterpolation.h
 *
 *  Created on: Jul 21, 2024
 *      Author: Ali Emad
 */

#ifndef COTS_OS_INC_LIB_LINEARINTERPOLATION_LINEARINTERPOLATION_H_
#define COTS_OS_INC_LIB_LINEARINTERPOLATION_LINEARINTERPOLATION_H_

/*
 * Notes:
 * 		-	"piXArr": contains 'x' values sorted in an ascending order.
 * 		-	"piYArr": contains 'y' values sorted in an ascending order.
 * 		-	"uiLen": number of samples stored in "piXArr" and "piYArr", must be at least 2.
 * 		-	"iX": value of 'x' for which the interpolation is to be made.
 *
 * 		-	return: interpolated value of 'y'.
 */
int32_t iLIB_LinearInterpolation_interpolate(
		int32_t* piXArr,
		int32_t* piYArr,
		uint32_t uiLen,
		int32_t iX
	);



#endif /* COTS_OS_INC_LIB_LINEARINTERPOLATION_LINEARINTERPOLATION_H_ */
