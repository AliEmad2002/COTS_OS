/*
 * OExtendShiftRegister.c
 *
 *  Created on: Feb 29, 2024
 *      Author: Ali Emad
 */

/*	LIB	*/
#include "stdint.h"

/*	RTOS	*/
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

/*	MCAL	*/
#include "MCAL_Port/Port_SPI.h"
#include "MCAL_Port/Port_DIO.h"

/*	HAL	*/
#include "HAL/SPI/SPI.h"

/*	SELF	*/
#include "HAL/IOExtend/OExtendShiftRegister.h"


static void  vSpiTransmit(xHOS_OExtendShiftRegister_t* pxHandle)
{
	if (pxHandle->ucSpiUnitNumber != 255)
	{
		ucHOS_SPI_takeMutex(pxHandle->ucSpiUnitNumber, portMAX_DELAY);

		vHOS_SPI_setByteDirection(	pxHandle->ucSpiUnitNumber,
									ucHOS_SPI_BYTE_DIRECTION_MSBYTE_FIRST	);

		vHOS_SPI_send(	pxHandle->ucSpiUnitNumber,
						(int8_t*)&pxHandle->uiCurrentOutputWord,
						pxHandle->ucRegisterSizeInBytes	);

		ucHOS_SPI_blockUntilTransferComplete(pxHandle->ucSpiUnitNumber, portMAX_DELAY);

		vHOS_SPI_releaseMutex(pxHandle->ucSpiUnitNumber);
	}

	else
	{
		for (int8_t reg = pxHandle->ucRegisterSizeInBytes - 1; reg >= 0; reg--)
		{
			for (int8_t i = 7; i >= 0; i--)
			{
				uint8_t ucState = (pxHandle->uiCurrentOutputWord >> (i + reg * 8)) & 1;
				vPORT_DIO_WRITE_PIN(pxHandle->ucDataPort, pxHandle->ucDataPin, ucState);

				vTaskDelay(pdMS_TO_TICKS(10));

				vPORT_DIO_WRITE_PIN(pxHandle->ucSckPort, pxHandle->ucSckPin, 1);

				vTaskDelay(pdMS_TO_TICKS(10));

				vPORT_DIO_WRITE_PIN(pxHandle->ucSckPort, pxHandle->ucSckPin, 0);

				vTaskDelay(pdMS_TO_TICKS(10));
			}
		}

	}
}


void vHOS_OExtendShiftRegister_init(xHOS_OExtendShiftRegister_t* pxHandle)
{
	/*	Initialize mutex	*/
	pxHandle->xMutex = xSemaphoreCreateMutexStatic(&pxHandle->xMutexStatic);
	xSemaphoreGive(pxHandle->xMutex);

	/*	Initialize latch pin	*/
	vPort_DIO_initPinOutput(pxHandle->ucLatchPort, pxHandle->ucLatchPin);
	vPORT_DIO_WRITE_PIN(pxHandle->ucLatchPort, pxHandle->ucLatchPin, 0);

	/*	Initialize SCK and data pins (only if HW SPI is not used)	*/
	if (pxHandle->ucSpiUnitNumber == 255)
	{
		vPort_DIO_initPinOutput(pxHandle->ucSckPort, pxHandle->ucSckPin);
		vPORT_DIO_WRITE_PIN(pxHandle->ucSckPort, pxHandle->ucSckPin, 0);

		vPort_DIO_initPinOutput(pxHandle->ucDataPort, pxHandle->ucDataPin);
		vPORT_DIO_WRITE_PIN(pxHandle->ucDataPort, pxHandle->ucDataPin, 0);
	}

	/*	Output word is initially zero	*/
	vHOS_OExtendShiftRegister_writePort(pxHandle, 0xFFFFFFFF, 0);
}

void vHOS_OExtendShiftRegister_writePin(
		xHOS_OExtendShiftRegister_t* pxHandle,
		uint8_t ucPinNumber,
		uint8_t ucVal	)
{
	/*	Lock mutex	*/
	xSemaphoreTake(pxHandle->xMutex, portMAX_DELAY);

	/*	Write Latch pin low	*/
	vPORT_DIO_WRITE_PIN(pxHandle->ucLatchPort, pxHandle->ucLatchPin, 0);

	/*	Edit stored current value of the port	*/
	if (ucVal)
		pxHandle->uiCurrentOutputWord |= (1ul << ucPinNumber);
	else
		pxHandle->uiCurrentOutputWord &= (~(1ul << ucPinNumber));

	/*	SPI transmit	*/
	vSpiTransmit(pxHandle);

	/*	Write Latch pin high	*/
	vPORT_DIO_WRITE_PIN(pxHandle->ucLatchPort, pxHandle->ucLatchPin, 1);

	/*	Unlock mutex	*/
	xSemaphoreGive(pxHandle->xMutex);
}

void vHOS_OExtendShiftRegister_writePort(
		xHOS_OExtendShiftRegister_t* pxHandle,
		uint32_t uiMask,
		uint32_t uiVal	)
{
	/*	Lock mutex	*/
	xSemaphoreTake(pxHandle->xMutex, portMAX_DELAY);

	/*	Write Latch pin low	*/
	vPORT_DIO_WRITE_PIN(pxHandle->ucLatchPort, pxHandle->ucLatchPin, 0);

	/*	Edit stored current value of the port	*/
	pxHandle->uiCurrentOutputWord &= ~(uiMask);                                        \
	pxHandle->uiCurrentOutputWord |= (uiVal);

	/*	SPI transmit	*/
	vSpiTransmit(pxHandle);

	/*	Write Latch pin high	*/
	vPORT_DIO_WRITE_PIN(pxHandle->ucLatchPort, pxHandle->ucLatchPin, 1);

	/*	Unlock mutex	*/
	xSemaphoreGive(pxHandle->xMutex);
}
