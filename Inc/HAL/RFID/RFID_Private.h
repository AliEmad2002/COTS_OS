/*
 * RFID_Private.h
 *
 *  Created on: Oct 27, 2023
 *      Author: Ali Emad
 */

#ifndef COTS_OS_INC_HAL_RFID_RFID_PRIVATE_H_
#define COTS_OS_INC_HAL_RFID_RFID_PRIVATE_H_

typedef struct{
	/*	PRIVATE	*/
	uint8_t ucSOF;
	char pcIDStr[10];
	char pcCheckSumStr[2];
	uint8_t ucEOF;
}xHOS_RFID_RDM3600_Frame_t;

typedef struct{
	/*	PRIVATE	*/
	uint8_t ucSOF;
	char pcIDStr[10];
	char pcCheckSumStr[1];
	uint8_t ucEOF;
}xHOS_RFID_RF125PS_Frame_t;

#define xHOS_RFID_LARGER_FRAME_SZ	xHOS_RFID_RDM3600_Frame_t



#endif /* COTS_OS_INC_HAL_RFID_RFID_PRIVATE_H_ */
