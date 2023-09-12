/*
 * SDC_Test1.c
 *
 *  Created on: May 1, 2023
 *      Author: ali20
 *
 *  Do not forget to un-comment the commented lines in functions:
 *  	-	SDC_voidKeepTryingWriteStream()
 *  	-	SDC_voidKeepTryingReadStream()
 *
 *  Reads and prints from file named "FILE.NC", line by line.
 */

#if 0
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

	SDC_t sd;
	SDC_voidKeepTryingInitConnection(&sd, 1, SPI_UnitNumber_1, SD_CS_PIN, SD_AFIO_MAP);
	SDC_voidKeepTryingInitPartition(&sd);

	SD_Stream_t stream;
	SDC_voidKeepTryingOpenStream(&stream, &sd, "FILE.NC");

	char line[1024];

	while(SDC_u8IsThereNextLine(&stream))
	{
		SDC_voidGetNextLine(&stream, line, 128);
		trace_printf("%s\n", line);
	}

	trace_printf("Program done!\n");
	trace_printf("numberOfFails      = %d\n", numberOfFails);
	trace_printf("numberOfHardResets = %d\n", numberOfHS);

	while(1);

	return 0;
}

#endif	/*	0	*/
