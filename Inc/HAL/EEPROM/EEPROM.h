/*
 * EEPROM.h
 *
 *  Created on: Mar 1, 2024
 *      Author: Ali Emad
 *
 * So-far, this driver interfaces external I2C connected EEPROMs.
 */

#ifndef COTS_OS_INC_HAL_EEPROM_EEPROM_H_
#define COTS_OS_INC_HAL_EEPROM_EEPROM_H_

typedef struct{
	/*	PUBLIC	*/
	uint8_t ucI2cUnitNumber;
	uint8_t ucI2cAddress;
	uint8_t ucWriteEnablePort;
	uint8_t ucWriteEnablePin;
	uint32_t uiSizeInBytes;
}xHOS_EEPROM_t;

/*
 * Initializes handle.
 *
 * Notes:
 * 		-	All public parameters must be initialized first.
 */
void vHOS_EEPROM_init(xHOS_EEPROM_t* pxHandle);

/*
 * Write data buffer to selectable address at EEPROM.
 *
 * Notes:
 * 		-	Handle's mutex must be locked first by the calling task.
 *
 * 		-	If function's parameters exceed maximum defined size of the handle,
 * 			or EEPROM stops responding, function returns 0.
 *
 * 		-	Otherwise, function returns 1.
 */
uint8_t ucHOS_EEPROM_write(	xHOS_EEPROM_t* pxHandle,
							const uint8_t* pucBuffer,
							uint32_t uiBufferSz,
							uint32_t uiStartingAddress,
							TickType_t xTimeout	);

/*
 * Reads data from selectable address at EEPROM to a buffer in RAM.
 *
 * Notes:
 * 		-	Handle's mutex must be locked first by the calling task.
 *
 * 		-	If function's parameters exceed maximum defined size of the handle,
 * 			or EEPROM stops responding, function returns 0.
 *
 * 		-	Otherwise, function returns 1.
 */
uint8_t ucHOS_EEPROM_read(	xHOS_EEPROM_t* pxHandle,
							uint8_t* pucBuffer,
							uint32_t uiBufferSz,
							uint32_t uiStartingAddress,
							TickType_t xTimeout	);

#endif /* COTS_OS_INC_HAL_EEPROM_EEPROM_H_ */
