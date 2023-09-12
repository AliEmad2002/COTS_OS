/*
 * SDC_Test0.c
 *
 *  Created on: May 1, 2023
 *      Author: ali20
 *
 *	SD-card must have two files "S0.BIN", "S1.BIN" in its root directory,
 *	each of 10KB, the test will try copying S0 to S1, but not copying bytes
 *	consecutively, it copies them randomly to increase buffer miss chances.
 */

#if 0

//#define RAND_TEST
#define NORMAL_TEST

/*	LIB	*/
#include "Std_Types.h"
#include "Bit_Math.h"
#include <diag/trace.h>
#include <stdlib.h>

/*	MCAL	*/
#include "RCC_interface.h"
#include "GPIO_interface.h"
#include "STK_interface.h"

/*	HAL	*/
#include "SDC_interface.h"

#define SD_SPI_UNIT_NUMBER		SPI_UnitNumber_1
#define SD_CS_PIN				GPIO_Pin_B0
#define SD_AFIO_MAP				0

static u8 flagArr[1280] = {0};

u8 all_flags_set()
{
	for (u16 i = 0; i < 1280; i++)
	{
		if (flagArr[i] != 0xFF)
			return 0;
	}
	return 1;
}

ALWAYS_INLINE_STATIC u8 get_flag(u32 offset)
{
	return GET_BIT(flagArr[offset / 8], offset % 8);
}

ALWAYS_INLINE_STATIC u8 set_flag(u32 offset)
{
	return SET_BIT(flagArr[offset / 8], offset % 8);
}

u32 get_rand_unused_offset()
{
	static u32 i = 0;
	return i++;
//	while(1)
//	{
//		u32 offset = ((u32)rand()) % 10240u;
//		if (get_flag(offset) == 0)
//			return offset;
//	}
}

u32 numberOfFails = 0;
u32 numberOfHS = 0;

void init_mcal()
{
	/**	Init system clock	**/
	RCC_voidSysClockInit();

	/**	Enable GPIO ports	**/
	RCC_voidEnablePeripheralClk(RCC_Bus_APB2, RCC_PERIPHERAL_AFIO);
	RCC_voidEnablePeripheralClk(RCC_Bus_APB2, RCC_PERIPHERAL_IOPA);
	RCC_voidEnablePeripheralClk(RCC_Bus_APB2, RCC_PERIPHERAL_IOPC);
	RCC_voidEnablePeripheralClk(RCC_Bus_APB2, RCC_PERIPHERAL_IOPB);

	/**	Init SysTick	**/
	/*	init	*/
	STK_voidInit();

	/*	enable	*/
	STK_voidEnableSysTick();

	/*	enable ovfCounter	*/
	STK_voidStartTickMeasure(STK_TickMeasureType_OverflowCount);
}

int main(void)
{
	/*	init MCAL	*/
	init_mcal();

	srand(0);

	SDC_t sd;
	SDC_voidKeepTryingInitConnection(&sd, 1, SPI_UnitNumber_1, SD_CS_PIN, SD_AFIO_MAP);
	SDC_voidKeepTryingInitPartition(&sd);

	SD_Stream_t s0, s1;
	SDC_voidKeepTryingOpenStream(&s0, &sd, "S0.BIN");
	SDC_voidKeepTryingOpenStream(&s1, &sd, "S1.BIN");

	u8 byte;


	#ifdef NORMAL_TEST

	for (u32 i = 0; i < s0.sizeActual; i++)
	{
		SDC_voidKeepTryingReadStream(&s0, i, &byte, 1);
		SDC_voidKeepTryingWriteStream(&s1, i, &byte, 1);

		if (i % (s0.sizeActual / 100) == 0)
		{
			static u32 p = 0;
			trace_printf("%d\n", p++);
		}
	}

	#endif

	#ifdef RAND_TEST
	u32 offset;
	u32 doneCount = 0;

	while(!all_flags_set())
	{
		offset = get_rand_unused_offset();

		SDC_voidKeepTryingReadStream(&s0, offset, &byte, 1);
		SDC_voidKeepTryingWriteStream(&s1, offset, &byte, 1);

		set_flag(offset);

		doneCount++;

		if (doneCount % 102 == 0)
		{
			static u32 i = 0;
			trace_printf("%d\n", i++);
		}
	}
	#endif

	SDC_voidKeepTryingSaveStream(&s0);
	SDC_voidKeepTryingSaveStream(&s1);

	trace_printf("Program done!\n");
	trace_printf("numberOfFails      = %d\n", numberOfFails);
	trace_printf("numberOfHardResets = %d\n", numberOfHS);

	while(1);

	return 0;
}

#endif	/*	0	*/
