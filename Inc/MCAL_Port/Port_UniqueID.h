/*
 * Port_UniqueID.h
 *
 *  Created on: Nov 30, 2023
 *      Author: Ali Emad
 */

#ifndef COTS_OS_SRC_MCAL_PORT_PORT_UNIQUEID_H_
#define COTS_OS_SRC_MCAL_PORT_PORT_UNIQUEID_H_


#define uiPORT_UNIQUEID_SIZE_IN_BYTES		12

/*
 * Reads device's unique ID into the array "pucID". array allocation is user's
 * responsibility.
 */
void vPort_UniqueID_get(uint8_t* pucID);


#endif /* COTS_OS_SRC_MCAL_PORT_PORT_UNIQUEID_H_ */
