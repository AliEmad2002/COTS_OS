/*
 * Bit_Math.h
 *
 *  Created on: Jun 10, 2023
 *      Author: Ali Emad
 */

#ifndef HAL_OS_LIB_BIT_MATH_H_
#define HAL_OS_LIB_BIT_MATH_H_

#include "Std_Types.h"

/*	evaluates i-th bit of x	*/
#define ucGET_BIT(x, i) (((x) >> (i)) & 1)

/*
 * evaluates i-th nibble of "arr". (Follows little endianness)
 *
 * Notes:
 * 		-	"arr" must be a pointer to "uint8_t".
 */
#define ucGET_NIBBLE(arr, i) (((i)%2) ? ((arr)[(i)/2] >> 4) : ((arr)[(i)/2] & 0x0F))

/*
 * writes i-th nibble of "arr" with value of "v". (Follows little endianness)
 *
 * Notes:
 * 		-	"arr" must be a pointer to "uint8_t".
 * 		-	"v" must be less than or equal to 0x0F.
 */
#define vWRT_NIBBLE(arr, i, v)                                 \
{                                                              \
	if ((i) % 2)                                               \
	{                                                          \
		(arr)[(i)/2] = ((arr)[(i)/2] & 0x0F) | ((v) << 4);     \
	}                                                          \
	else                                                       \
	{                                                          \
		(arr)[(i)/2] = ((arr)[(i)/2] & 0xF0) | (v);            \
	}                                                          \
}

/******************************************************************************
 * LEGACY CODE (For MCAL files)
 *****************************************************************************/
#define SET_BIT(reg, n)					((reg) |= 1<<(n))

#define CLR_BIT(reg, n)					((reg) &= ~(1<<(n)))

#define TGL_BIT(reg, n)					((reg) ^= 1<<(n))

#define GET_BIT(reg, n)					(((reg)>>(n)) & 1)

#define WRT_BIT(reg, n, val)			((val) ? SET_BIT((reg), (n)) : CLR_BIT((reg), (n)))

/*	2^n	*/
#define POW_TWO(n)						(1 << (n))

/*	maximum of n-bits (2^n - 1) */
#define MAX_N_BITS(n)					(POW_TWO((n)) - 1)

#define GET_REG_SEC(reg, start, len)	(((reg) >> (start)) & (POW_TWO((len)) - 1))

/*
 * Edit a block of bytes in a register to a new value
 * reg:		the register to be edited.
 * start:	number of the bit to start editing upwards from.
 * val:		value that is going to replace whatever is in "reg" starting from "start" bit.
 * len:		length of val.
 */
static inline void BitMath_voidEditRegister(u32* regPtr, u8 start, u32 val, u8 len)
{
	*regPtr = (*regPtr & ~((POW_TWO(len)-1) << start)) | (val << start);
}

#define EDT_REG(reg, start, val, len)	BitMath_voidEditRegister((u32*)&(reg), start, val, len)

#define GET_WORD_AT(basePtr, offset)	\
	(*((u32*)((u32)(basePtr) + (offset) * 4)))

#endif /* HAL_OS_LIB_BIT_MATH_H_ */
