/*
 * Port_UniqueID.h
 *
 *  Created on: Nov 30, 2023
 *      Author: Ali Emad
 */

/*	Target checking	*/
#include "MCAL_Port/Port_Target.h"
#ifdef MCAL_PORT_TARGET_STM32F103C8T6


#ifndef COTS_OS_SRC_MCAL_PORT_PORT_UNIQUEID_H_
#define COTS_OS_SRC_MCAL_PORT_PORT_UNIQUEID_H_


#define uiPORT_UNIQUEID_SIZE_IN_BYTES		12

/*
 * Reads device's unique ID into the array "pucID". array allocation is user's
 * responsibility.
 */
void vPort_UniqueID_get(uint8_t* pucID);


#endif /* COTS_OS_SRC_MCAL_PORT_PORT_UNIQUEID_H_ */


#endif /* Target checking */