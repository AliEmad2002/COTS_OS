/*
 * CRC.h
 *
 *  Created on: Aug 16, 2023
 *      Author: Ali Emad
 */

#ifndef COTS_OS_INC_LIB_CRC_CRC_H_
#define COTS_OS_INC_LIB_CRC_CRC_H_

/*
 * Returns CRC-7 of a given array.
 */
uint8_t ucLIB_CRC_getCrc7(uint8_t* pucArr, uint32_t uiLen);

/*
 * Returns CRC-16 of a given array.
 */
uint16_t usLIB_CRC_getCrc16(uint8_t* pucArr, uint32_t uiLen);


#endif /* COTS_OS_INC_LIB_CRC_CRC_H_ */
