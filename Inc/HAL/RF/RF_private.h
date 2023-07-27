/*
 * RF_private.h
 *
 *  Created on: Jul 26, 2023
 *      Author: Ali Emad
 */

#ifndef COTS_OS_INC_HAL_RF_RF_PRIVATE_H_
#define COTS_OS_INC_HAL_RF_RF_PRIVATE_H_


/*	Frame format	*/
typedef struct{
	uint8_t ucEOF;
	uint8_t ucCRC0;
	uint8_t ucCRC1;
	uint8_t ucData[15];
	uint8_t ucDataLen;
	uint8_t ucSrcAddress;
	uint8_t ucSeqNum : 7;
	uint8_t ucIsAck : 1;
	uint8_t ucDestAddress;
	uint8_t ucSOF;
}xHOS_RF_Data_t;










#endif /* COTS_OS_INC_HAL_RF_RF_PRIVATE_H_ */
