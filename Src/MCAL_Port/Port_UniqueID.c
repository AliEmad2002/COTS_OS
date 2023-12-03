/*
 * Port_UniqueID.c
 *
 *  Created on: Nov 30, 2023
 *      Author: Ali Emad
 */

/*	LIB	*/
#include "stdint.h"

/*	MCAL	*/
#include "stm32f1xx_ll_utils.h"

/*	SELF	*/
#include "MCAL_Port/Port_UniqueID.h"

/*
 * See header for info.
 */
void vPort_UniqueID_get(uint8_t* pucID)
{
	*((uint32_t*)&pucID[0]) = LL_GetUID_Word0();
	*((uint32_t*)&pucID[4]) = LL_GetUID_Word1();
	*((uint32_t*)&pucID[8]) = LL_GetUID_Word2();
}
