/*
 * ApproxTrig.h
 *
 *  Created on: Sep 28, 2023
 *      Author: Ali Emad
 *
 * References:
 * 		-	https://youtu.be/1xlCVBIF_ig?si=cBxw9hLeZqK-3o83
 */

#ifndef COTS_OS_INC_LIB_APPROXTRIG_H_
#define COTS_OS_INC_LIB_APPROXTRIG_H_

/*
 * This function evaluates the following formula:
 *  f(x) ≈ 1000 * sin(2πx / 1000)
 *
 * Using only integer operations.
 *
 * Notes:
 * 		-	To get sine of a negative value, you can add multiples of 1000 to it
 * 			until it becomes positive (similar to adding multiples of 2π).
 */
int32_t iLIB_ApproxTrig_Sin1(uint64_t ulX);

/*
 * This function evaluates the following formula:
 *  f(x) ≈ 1000000 * sin(2πx / 1000000)
 *
 * Using only integer operations.
 *
 * Notes:
 * 		-	To get sine of a negative value, you can add multiples of 1000000 to it
 * 			until it becomes positive (similar to adding multiples of 2π).
 */
int64_t lLIB_ApproxTrig_Sin2(uint64_t ulX);

/*
 * This function evaluates the following formula:
 *  f(x) ≈ atan(x)
 *
 * Using only integer operations.
 *
 * Notes:
 * 		-	This function is valid only when x is in the range: (-1, 1).
 */
float fLIB_ApproxTrig_Atan1(float fX);

/*
 * This function evaluates the following formula:
 *  f(x) ≈ 1000 * atan(x / 1000)
 *
 * Using only integer operations.
 *
 * Notes:
 * 		-	This function is valid only when x is in the range: (-1000, 1000).
 *		-	Maximum output error when tested: 0.003 rad.
 */
int32_t iLIB_ApproxTrig_Atan2(int32_t iX);




#endif /* COTS_OS_INC_LIB_APPROXTRIG_H_ */
