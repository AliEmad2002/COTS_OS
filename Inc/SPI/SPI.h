/*
 * SPI.h
 *
 *  Created on: Jun 12, 2023
 *      Author: Ali Emad
 */

#ifndef HAL_OS_INC_SPI_SPI_H_
#define HAL_OS_INC_SPI_SPI_H_

#include "Inc/SPI/SPIConfig.h"


void vHOS_SPI_send(uint8_t ucUnitNumber, uint8_t* pucArr, uint32_t uiSize);


#endif /* HAL_OS_INC_SPI_SPI_H_ */
