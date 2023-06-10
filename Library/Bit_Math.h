/*
 * Bit_Math.h
 *
 *  Created on: Jun 10, 2023
 *      Author: Ali Emad
 */

#ifndef HAL_OS_LIB_BIT_MATH_H_
#define HAL_OS_LIB_BIT_MATH_H_


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

#endif /* HAL_OS_LIB_BIT_MATH_H_ */
