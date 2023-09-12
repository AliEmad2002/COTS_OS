///*
// * SDC_program.c
// *
// *  Created on: Apr 24, 2023
// *      Author: Ali Emad
// */
//
///*	LIB	*/
//#include "Std_Types.h"
//#include "Bit_Math.h"
//#include "Delay_interface.h"
//#include "diag/trace.h"
//#include <ctype.h>
//#include <cmsis_gcc.h>
//#include <HAL/SDC/SDC.h>
//#include <string.h>
//
///*	MCAL	*/
//#include "NVIC_interface.h"
//#include "STK_interface.h"
//#include "SPI_interface.h"
//#include "GPIO_interface.h"
//
///*	SELF	*/
//#include "SDC_config.h"
//#include "SDC_Private.h"
//#include "SDC_CRC.h"
//
//
//
//
///*******************************************************************************
// * Search:
// ******************************************************************************/
/////*	Searches for an array of bytes in the whole card (debug only)	*/
////static s32 find_in_block(u8* block, u32 blockLen, u8* byteArr, u32 len)
////{
////	for (u32 i = 0; i < blockLen; i++)
////	{
////		u8 mismatch = 0;
////		for (u32 j = 0; j < len; j++)
////		{
////			if (block[i+j] != byteArr[j])
////			{
////				mismatch = 1;
////				break;
////			}
////		}
////		if (mismatch == 0)
////			return i;
////	}
////
////	return -1;
////}
////
////static void search_whole(SDC_t* sdc, u8* byteArr, u32 len, s32* blockIndex, s32* indexInBlock)
////{
////	for (u8 i = 0; i < 200; i++)
////		trace_printf("#");
////	trace_printf("\n");
////
////	u8 successfull;
////
////	/*	for every block in the SD-card	*/
////	for (u32 i = 0; i < 8388608; i++)
////	{
////		/*	read block	*/
////		successfull = SDC_u8ReadBlock(sdc, sdc->block, 0);
////		if (!successfull)
////		{
////			//__asm volatile ("bkpt 0");
////			//trace_printf("read failed. retrying\n");
////			i--;
////		}
////
////		/*	search in block	*/
////		*indexInBlock = find_in_block(sdc->block, 512, byteArr, len);
////		if (*indexInBlock != -1)
////		{
////			*blockIndex = i;
////			__asm volatile ("bkpt 0");
////			return;
////		}
////
////		//trace_printf("%d\n", i);
////
////		if (i % (83886 / 2) == 0)
////		{
////			//static s32 p = 0;
////			//trace_printf("%d\n", p++);
////			trace_printf("=");
////		}
////	}
////
////	*indexInBlock = -1;
////	*blockIndex = -1;
////}
