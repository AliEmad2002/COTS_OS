/*
 * CRC.c
 *
 *  Created on: Aug 16, 2023
 *      Author: Ali Emad
 */

/*	LIB	*/
#include "stdint.h"

/*	SELF	*/
#include "LIB/CRC/CRC_Table.h"
#include "LIB/CRC/CRC.h"

/*
 * Check this:
 * http://www.rjhcoding.com/avrc-sd-interface-2.php
 * http://my-cool-projects.blogspot.com/2013/06/sd-card-crc-algorithm-explained-in.html
 * https://bits4device.wordpress.com/2017/12/16/sd-crc7-crc16-implementation/
 */
/**
 * Study notes about CRC calculation:
 * 		-	Simply CRC is performing a modulo operation between two
 * 			polynomials, data polynomial and generator polynomial.
 *
 * 		-	Polynomials are represented like the following examples:
 * 				0b1101   == x^3 + x^2 + 1
 * 				0b101001 == x^5 + x^3 + 1
 *
 * 		-	Binary polynomial division is very similar to binary numerical
 * 			division, except that it uses successive XOR'ing instead of
 * 			successive subtraction.
 *
 * 		-	let  :	D0 = D[0:7], D1 = D[8:15], D = D[0:15]
 *		 			C1 = CRC(D1)
 *
 * 			then : CRC(D) = CRC((C1 << 1) ^ D0)
 **/

/*******************************************************************************
 * API functions:
 ******************************************************************************/
/*
 * See header for info.
 */
uint8_t ucLIB_CRC_getCrc7(uint8_t* pucArr, uint32_t uiLen)
{
	uint8_t ucC = pucCrc7Table[0];

	for (int32_t i = (int32_t)uiLen - 1; i >= 0; i--)
	{
		ucC = pucCrc7Table[(ucC << 1) ^ pucArr[i]];
	}

	return ucC;
}

/*
 * See header for info.
 */
uint16_t usLIB_CRC_getCrc16(uint8_t* pucArr, uint32_t uiLen)
{
	uint16_t usC = pusCrc16Table[0];

	for (uint32_t i = 0; i < uiLen; i++)
	{
		usC = (usC<<8) ^ pusCrc16Table[(uint8_t)(usC>>8) ^ pucArr[i]];
	}

	return usC;
}
