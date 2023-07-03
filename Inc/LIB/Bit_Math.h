/*
 * Bit_Math.h
 *
 *  Created on: Jul 3, 2023
 *      Author: Ali Emad
 */

#ifndef COTS_OS_INC_LIB_BIT_MATH_H_
#define COTS_OS_INC_LIB_BIT_MATH_H_


#define ucGET_BIT(var, n) (((var) & (1 << (n))) >> (n))

#define vSET_BIT(var, n) (((var) & (1 << (n))) >> (n))


#endif /* COTS_OS_INC_LIB_BIT_MATH_H_ */
