/*
 * String.c
 *
 *  Created on: Sep 15, 2023
 *      Author: Ali Emad
 */

/*	LIB	*/
#include <stdint.h>
#include <string.h>
#include "LIB/Assert.h"

/*	SELF	*/
#include "LIB/String.h"

/*******************************************************************************
 * Helping functions / macros:
 ******************************************************************************/
static inline uint8_t ucCharToHex(char c)
{
	if ('0' <= c && c <= '9')
		return (uint8_t)(c - '0');

	if ('a' <= c && c <= 'f')
		return (uint8_t)((10 + c) - 'a');

	if ('A' <= c && c <= 'F')
		return (uint8_t)((10 + c) - 'A');

	/*	Otherwise, "c" is not valid	*/
	vLib_ASSERT(0, 1);
	return 0;
}

/*******************************************************************************
 * API functions:
 ******************************************************************************/
void vLIB_String_str2hex(char* pcStr, uint8_t* pucHex, uint32_t uiNumberOfBytes)
{
	for (uint32_t i = 0; i < uiNumberOfBytes; i++)
	{
		pucHex[i] = (ucCharToHex(pcStr[2*i]) << 4) | ucCharToHex(pcStr[2*i+1]);
	}
}
