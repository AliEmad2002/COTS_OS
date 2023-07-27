/*
 * RF.c
 *
 *  Created on: Jul 26, 2023
 *      Author: Ali Emad
 */


/*	LIB	*/
#include "stdint.h"

/*	MCAL (ported)	*/
#include "MCAL_Port/Port_Breakpoint.h"
#include "MCAL_Port/Port_DIO.h"

/*	OS	*/
#include "FreeRTOS.h"
#include "task.h"
#include "RTOS_PRI_Config.h"

/*	HAL	*/

/*	SELF	*/
#include "HAL/RF/RF.h"


/*******************************************************************************
 * Task function:
 ******************************************************************************/
static void vTask(void* pvParams)
{

}



/*******************************************************************************
 * API functions:
 ******************************************************************************/
/*	extern physical layer functions	*/
extern void xHOS_RFPhysical_init(xHOS_RF_t* pxHandle);

/*	See header for info	*/
void vHOS_RF_init(xHOS_RF_t* pxHandle)
{
	/*	Initialize physical layer	*/
	xHOS_RFPhysical_init(pxHandle);

	/*	Initialize task	*/

}

/*	See header for info	*/
void vHOS_RF_enable(xHOS_RF_t* pxHandle)
{

}

/*	See header for info	*/
void vHOS_RF_disable(xHOS_RF_t* pxHandle)
{

}

/*	See header for info	*/
void vHOS_RF_send(xHOS_RF_t* pxHandle, xHOS_RF_TransmissionInfo_t* pxInfo)
{

}













