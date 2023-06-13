/*
 * Bit_Math.h
 *
 *  Created on: Jun 10, 2023
 *      Author: Ali Emad
 */

#ifndef HAL_OS_LIB_BIT_MATH_H_
#define HAL_OS_LIB_BIT_MATH_H_

#include "Std_Types.h"

#define vSET_BIT(reg, n)				((reg) |= 1<<(n))

#define ucGET_BIT(x, i) 				(((x) >> (i)) & 1)

#define vCLR_BIT(reg, n)				((reg) &= ~(1<<(n)))

#define vTGL_BIT(reg, n)				((reg) ^= 1<<(n))

#define vWRT_BIT(reg, n, val)			((val) ? vSET_BIT((reg), (n)) : vCLR_BIT((reg), (n)))

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

/*	2^n	*/
#define uiPOW_TWO(n)						(1 << (n))

/*	maximum of n-bits (2^n - 1) */
#define uiMAX_N_BITS(n)					(uiPOW_TWO((n)) - 1)

#define uiGET_REG_SEC(reg, start, len)	(((reg) >> (start)) & (uiPOW_TWO((len)) - 1))

/*
 * Edit a block of bytes in a register to a new value
 * reg:		the register to be edited.
 * start:	number of the bit to start editing upwards from.
 * val:		value that is going to replace whatever is in "reg" starting from "start" bit.
 * len:		length of val.
 */
static inline void BitMath_voidEditRegister(u32* regPtr, u8 start, u32 val, u8 len)
{
	*regPtr = (*regPtr & ~((uiPOW_TWO(len)-1) << start)) | (val << start);
}

#define vEDT_REG(reg, start, val, len)	BitMath_voidEditRegister((u32*)&(reg), start, val, len)

#define uiGET_WORD_AT(basePtr, offset)	\
	(*((u32*)((u32)(basePtr) + (offset) * 4)))

#endif /* HAL_OS_LIB_BIT_MATH_H_ */
