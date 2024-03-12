/*
 * EEPROM.c
 *
 *  Created on: Mar 1, 2024
 *      Author: Ali Emad
 */


/*	LIB	*/
#include "stdint.h"

/*	RTOS	*/
#include "FreeRTOS.h"

/*	MCAL	*/
#include "MCAL_Port/Port_DIO.h"

/*	HAL	*/
#include "HAL/I2C/I2C.h"

/*	SELF	*/
#include "HAL/EEPROM/EEPROM.h"


/*******************************************************************************
 * API functions:
 ******************************************************************************/
/*
 * See header for info.
 */
void vHOS_EEPROM_init(xHOS_EEPROM_t* pxHandle)
{
	/*	Initialize write enable pin as output. And enable output	*/
	vPort_DIO_initPinOutput(pxHandle->ucWriteEnablePort, pxHandle->ucWriteEnablePin);
	vPORT_DIO_WRITE_PIN(pxHandle->ucWriteEnablePort, pxHandle->ucWriteEnablePin, 0);
}


/*
 * See header for info.
 */
uint8_t ucHOS_EEPROM_write(	xHOS_EEPROM_t* pxHandle,
							const uint8_t* pucBuffer,
							uint32_t uiBufferSz,
							uint32_t uiStartingAddress,
							TickType_t xTimeout	)
{
	/*	Write byte by byte (TODO: page write)	*/
	/*	TODO: Timeout	*/
	uint8_t pucFrame[3];
	uint16_t usAddress;

	xHOS_I2C_transreceiveParams_t xParams = {
			.ucUnitNumber = pxHandle->ucI2cUnitNumber,
			.usAddress = pxHandle->ucI2cAddress,
			.ucIs7BitAddress = 1,
			.pucTxArr = pucFrame,
			.uiTxSize = 3,
			.pucRxArr = NULL,
			.uiRxSize = 0
	};

	for (uint32_t i = 0; i < uiBufferSz; i++)
	{
		usAddress = uiStartingAddress + i;
		pucFrame[0] = usAddress >> 8;
		pucFrame[1] = usAddress & 0xFF;
		pucFrame[2] = pucBuffer[i];
		while(!ucHOS_I2C_masterTransReceive(&xParams));
		vTaskDelay(10);	/*	TODO	*/
	}

	return 1;
}


/*
 * See header for info.
 */
uint16_t usAddress;
uint8_t ucHOS_EEPROM_read(	xHOS_EEPROM_t* pxHandle,
							uint8_t* pucBuffer,
							uint32_t uiBufferSz,
							uint32_t uiStartingAddress,
							TickType_t xTimeout	)
{
	/*	Read byte by byte (TODO: page write)	*/
	/*	TODO: Timeout	*/
	uint8_t pucFrame[2];


	xHOS_I2C_transreceiveParams_t xParams = {
			.ucUnitNumber = pxHandle->ucI2cUnitNumber,
			.usAddress = pxHandle->ucI2cAddress,
			.ucIs7BitAddress = 1,
			.pucTxArr = pucFrame,
			.uiTxSize = 2,
			.uiRxSize = 1
	};

	for (uint32_t i = 0; i < uiBufferSz; i++)
	{
		/*	Dummy write (For address selecting)	*/
		usAddress = uiStartingAddress + i;
		pucFrame[0] = usAddress >> 8;
		pucFrame[1] = usAddress & 0xFF;

		/*	Read	*/
		xParams.pucRxArr = &pucBuffer[i];

		while(!ucHOS_I2C_masterTransReceive(&xParams));

//		vTaskDelay(1);	/*	TODO	*/
	}

	return 1;
}












