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
//	vLib_ASSERT(0, 1);
	return 255;
}

static inline char ucHexToChar(uint8_t halfByte)
{
	if (0 <= halfByte && halfByte <= 9)
		return (char)(halfByte + '0');

	if (10 <= halfByte && halfByte <= 15)
		return (char)((halfByte - 10) + 'A');

	/*	Otherwise, "halfByte" is not valid	*/
	vLib_ASSERT(0, 1);
	return 0;
}

/*******************************************************************************
 * API functions:
 ******************************************************************************/
/*
 * See header for info.
 */
void vLIB_String_str2hex(char* pcStr, uint8_t* pucHex, uint32_t uiNumberOfBytes)
{
	uint8_t LHByte, MHByte;
	uint8_t j = 0;
	for (int32_t i = 0; i < uiNumberOfBytes; i++)
	{
		LHByte = ucCharToHex(pcStr[2*i+1 + j]);
		MHByte = (ucCharToHex(pcStr[2*i + j]) << 4);

		if (LHByte == 255 || MHByte == 255)
		{
			j++;
			continue;
		}

		pucHex[i] = MHByte | LHByte;
	}
}

/*
 * See header for info.
 */
void vLIB_String_hex2str(uint8_t* pucHex, char* pcStr, uint32_t uiNumberOfBytes)
{
	for (uint32_t i = 0; i < uiNumberOfBytes; i++)
	{
		pcStr[2*i] = ucHexToChar(pucHex[i] >> 4);
		pcStr[2*i+1] = ucHexToChar(pucHex[i] & 0x0F);
	}

	pcStr[2*uiNumberOfBytes] = '\0';
}










