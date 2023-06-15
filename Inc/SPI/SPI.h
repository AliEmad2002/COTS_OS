/*
 * SPI.h
 *
 *  Created on: Jun 12, 2023
 *      Author: Ali Emad
 */

#ifndef HAL_OS_INC_SPI_SPI_H_
#define HAL_OS_INC_SPI_SPI_H_

/*******************************************************************************
 * Include dependencies:
 ******************************************************************************/
#include "Inc/SPI/SPIConfig.h"
#include "semphr.h"

/*******************************************************************************
 * API functions/macros
 ******************************************************************************/
#define ucHOS_SPI_BYTE_DIRECTION_MSBYTE_FIRST		0
#define ucHOS_SPI_BYTE_DIRECTION_LSBYTE_FIRST		1
void vHOS_SPI_setByteDirection(uint8_t ucUnitNumber, uint8_t ucByteDirection);

void vHOS_SPI_send(uint8_t ucUnitNumber, int8_t* pcArr, uint32_t uiSize);

/***********
 * Difference between "SpiTransferMutex" and "SpiUintMutex":
 * 		-	"SpiUintMutex" is taken by the task to not allow any other task use
 * 			the same unit in the same time.
 *
 * 		-	"SpiTransferMutex" is taken within inside the take that has already
 * 			took "SpiUintMutex" on start of a transfer, and would get released
 * 			by SPI driver as soon as SPI unit has completed the transfer.
 * 			This is done to assure that the  task using SPI unit does not start
 * 			another transfer while the previous one has not yet ended.
 ***********/
/*
 * Returns SPI transfer mutex handle
 */
SemaphoreHandle_t xHOS_SPI_getTransferMutexHandle(uint8_t ucUnitNumber);

/*
 * Returns SPI unit mutex handle
 */
SemaphoreHandle_t xHOS_SPI_getUnitMutexHandle(uint8_t ucUnitNumber);

#endif /* HAL_OS_INC_SPI_SPI_H_ */
