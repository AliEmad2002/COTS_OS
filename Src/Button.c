/*
 * Button.c
 *
 *  Created on: Jun 8, 2023
 *      Author: Ali Emad
 */

/*	LIB	*/
#include <stdint.h>

/*	FreeRTOS	*/
#include "FreeRTOS.h"
#include "task.h"

/*	MCAL (Ported)	*/
#include "Port/Port_DIO.h"

/*	SELF	*/
#include "Inc/Button/Button.h"

/*
 * If "configHOS_BUTTON_EN" was enabled, a task that uses this function will be
 * created in the "vHOS_init()".
 *
 * For further information about how HAL_OS drivers are managed, refer to the
 * repository's "README.md"
 */
void button_manager(void* pvParams)
{

}

/*
 * See header file for info.
 */
void vHOS_Button_init(	uint8_t ucPortNumber,
						uint8_t ucPinNumber,
						void (*pfCallback)(void),
						uint8_t ucPressedLevel,
						HOS_Button_t* xButtonHandle	)
{

}

/*
 * See header file for info.
 */
void vHOS_Button_delete(HOS_Button_t* xButtonHandle)
{

}
