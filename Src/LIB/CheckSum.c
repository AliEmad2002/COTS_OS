/*
 * CheckSum.c
 *
 *  Created on: Sep 15, 2023
 *      Author: Ali Emad
 */

#include <stdint.h>
#include "LIB/CheckSum.h"

uint8_t ucLIB_CheckSum_get8BitCheck(uint8_t* pucArr, uint32_t uiLen)
{
	uint8_t ucCheck = 0;

	for (uint32_t i = 0; i < uiLen; i++)
		ucCheck = ucCheck ^ pucArr[i];

	return ucCheck;
}
