/*
 * HAL_OS.c
 *
 *  Created on: Jun 9, 2023
 *      Author: Ali Emad
 */

/*	LIB	*/
#include <stdint.h>

/*	FreeRTOS	*/
#include "FreeRTOS.h"
#include "task.h"

/*	MCAL (Ported)	*/
#include "Port/Port_DIO.h"
#include "Port/Port_Print.h"
#include "Port/Port_Breakpoint.h"

/*	HAL_OS drivers	*/
#include "Inc/Button/Button.h"

/*	SELF	*/
#include "Inc/HAL_OS.h"

/*	Stack size configuration	*/
#define uiBUTTON_STACK_SIZE_IN_WORDS	configMINIMAL_STACK_SIZE

/*	Drivers' stacks and task handles	*/
static StackType_t puxButtonDriverStack[uiBUTTON_STACK_SIZE_IN_WORDS];

static StaticTask_t xButtonDriverTask;

/*	Drivers' task functions	*/
extern void vButton_manager(void* pvParams);

BaseType_t xHOS_init(void)
{
	TaskHandle_t createdHandle;

#if configHOS_BUTTON_EN
	createdHandle = xTaskCreateStatic(	vButton_manager,
										"Buttons",
										uiBUTTON_STACK_SIZE_IN_WORDS,
										NULL,
										configNORMAL_TASK_PRIORITY,
										puxButtonDriverStack,
										&xButtonDriverTask	);
	if (createdHandle == NULL)
		return pdFAIL;
#endif	/*	configHOS_BUTTON_EN	*/



	return pdPASS;
}




















