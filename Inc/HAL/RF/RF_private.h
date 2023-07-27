/*
 * RF_private.h
 *
 *  Created on: Jul 26, 2023
 *      Author: Ali Emad
 */

#ifndef COTS_OS_INC_HAL_RF_RF_PRIVATE_H_
#define COTS_OS_INC_HAL_RF_RF_PRIVATE_H_


#include "HAL/RF/RF_config.h"

/*	Frame format	*/
typedef struct{
	uint8_t ucEOF;
	uint8_t ucCRC0;
	uint8_t ucCRC1;
	uint8_t pucData[uiRF_DATA_BYTES_PER_FRAME];
	uint8_t ucSrcAddress;
	uint8_t ucStatus : 7;
	uint8_t ucIsAck : 1;
	uint8_t ucDestAddress;
	uint8_t ucSOF;
}xHOS_RF_Frame_t;

#define ucRF_SOF		(0b10101111)
#define ucRF_EOF		(0b11111111)








#endif /* COTS_OS_INC_HAL_RF_RF_PRIVATE_H_ */
