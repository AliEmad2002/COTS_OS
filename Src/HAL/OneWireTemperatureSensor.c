/*
 * OneWireTemperatureSensor.c
 *
 *  Created on: Jun 22, 2024
 *      Author: Ali Emad
 */

/*	LIB	*/
#include "stdint.h"

/*	MCAL	*/
#include "MCAL_Port/Port_GPIO.h"
#include "MCAL_Port/Port_DIO.h"

/*	HAL	*/
#include "HAL/HardwareDelay/HardwareDelay.h"

/*	SELF	*/
#include "HAL/OneWireTemperatureSensor/OneWireTemperatureSensor.h"

/*******************************************************************************
 * Static (private) functions:
 ******************************************************************************/
/*
 * Notes:
 * 	-	Outputs reset pulse, and checks for presence pulse.
 * 	-	Returns 1 if presence pulse took place, 0 otherwise.
 * 	-	"pxHwDelayHandle" must be initially locked.
 */
uint8_t ucInitializationSequence(xHOS_OneWireTemperatureSensor_t* pxHandle)
{
	/*	Wait 1us to assure end of previous bit	*/
	vHOS_HardwareDelay_delayUs(pxHandle->pxHwDelayHandle, 1);

	/*	Set the communication line low (reset pulse) for 480us	*/
	vPort_DIO_initPinOutputOpenDrain(pxHandle->ucPort, pxHandle->ucPin);
	vPORT_DIO_WRITE_PIN(pxHandle->ucPort, pxHandle->ucPin, 0);
	vHOS_HardwareDelay_delayUs(pxHandle->pxHwDelayHandle, 480);

	/*	Release the communication line and wait for 60us	*/
	vPORT_DIO_WRITE_PIN(pxHandle->ucPort, pxHandle->ucPin, 1);
	vHOS_HardwareDelay_delayUs(pxHandle->pxHwDelayHandle, 60);

	/*	Sense the presence pulse	*/
	vPort_DIO_initPinInput(pxHandle->ucPort, pxHandle->ucPin, 0);
	uint8_t ucPresence = ucPORT_DIO_READ_PIN(pxHandle->ucPort, pxHandle->ucPin);

	/*	Wait for another 420us (According to figure 15 in DS18B20 datasheet)	*/
	vHOS_HardwareDelay_delayUs(pxHandle->pxHwDelayHandle, 420);

	if (ucPresence == 0)
		return 1;
	else
		return 0;
}

/*
 * Notes:
 * 	-	"pxHwDelayHandle" must be initially locked.
 */
void vWriteBit0(xHOS_OneWireTemperatureSensor_t* pxHandle)
{
	/*	Wait 1us to assure end of previous bit	*/
	vHOS_HardwareDelay_delayUs(pxHandle->pxHwDelayHandle, 1);

	/*	Set the communication line low (reset pulse) for 60us	*/
	vPort_DIO_initPinOutputOpenDrain(pxHandle->ucPort, pxHandle->ucPin);
	vPORT_DIO_WRITE_PIN(pxHandle->ucPort, pxHandle->ucPin, 0);
	vHOS_HardwareDelay_delayUs(pxHandle->pxHwDelayHandle, 60);

	/*	Release the communication line	*/
	vPORT_DIO_WRITE_PIN(pxHandle->ucPort, pxHandle->ucPin, 1);
}

/*
 * Notes:
 * 	-	"pxHwDelayHandle" must be initially locked.
 */
void vWriteBit1(xHOS_OneWireTemperatureSensor_t* pxHandle)
{
	/*	Wait 1us to assure end of previous bit	*/
	vHOS_HardwareDelay_delayUs(pxHandle->pxHwDelayHandle, 1);

	/*	Set the communication line low (reset pulse) for 1us	*/
	vPort_DIO_initPinOutputOpenDrain(pxHandle->ucPort, pxHandle->ucPin);
	vPORT_DIO_WRITE_PIN(pxHandle->ucPort, pxHandle->ucPin, 0);
	vHOS_HardwareDelay_delayUs(pxHandle->pxHwDelayHandle, 1);

	/*	Release the communication line, keep it released for 60us	*/
	vPORT_DIO_WRITE_PIN(pxHandle->ucPort, pxHandle->ucPin, 1);
	vHOS_HardwareDelay_delayUs(pxHandle->pxHwDelayHandle, 60);
}

/*
 * Notes:
 * 	-	"pxHwDelayHandle" must be initially locked.
 */
uint8_t ucReadBit(xHOS_OneWireTemperatureSensor_t* pxHandle)
{
	/*	Wait 1us to assure end of previous bit	*/
	vHOS_HardwareDelay_delayUs(pxHandle->pxHwDelayHandle, 1);

	/*	Set the communication line low (reset pulse) for 1us	*/
	vPort_DIO_initPinOutputOpenDrain(pxHandle->ucPort, pxHandle->ucPin);
	vPORT_DIO_WRITE_PIN(pxHandle->ucPort, pxHandle->ucPin, 0);
	vHOS_HardwareDelay_delayUs(pxHandle->pxHwDelayHandle, 1);

	/*	Release the communication line, and wait for 15us	*/
	vPORT_DIO_WRITE_PIN(pxHandle->ucPort, pxHandle->ucPin, 1);
	vHOS_HardwareDelay_delayUs(pxHandle->pxHwDelayHandle, 15);

	/*	Sense sensor's output	*/
	vPort_DIO_initPinInput(pxHandle->ucPort, pxHandle->ucPin, 0);
	uint8_t ucBit = ucPORT_DIO_READ_PIN(pxHandle->ucPort, pxHandle->ucPin);

	/*	Wait for 45us	*/
	vHOS_HardwareDelay_delayUs(pxHandle->pxHwDelayHandle, 45);

	return ucBit;
}

/*	Writes one byte to the sensor (used in debug)	*/
void vWriteByte(xHOS_OneWireTemperatureSensor_t* pxHandle, uint8_t ucByte)
{
	for (uint8_t i = 0; i < 8; i++)
	{
		if (((ucByte >> i) & 1) == 1)
			vWriteBit1(pxHandle);
		else
			vWriteBit0(pxHandle);
	}
}

/*	Reads one byte from the sensor (used in debug)	*/
uint8_t ReadByte(xHOS_OneWireTemperatureSensor_t* pxHandle)
{
	uint8_t ucByte = 0;

	for (uint8_t i = 0; i < 8; i++)
	{
		if (ucReadBit(pxHandle));
			ucByte = ucByte | (1 << i);
	}

	return ucByte;
}

/*******************************************************************************
 * API functions:
 ******************************************************************************/
/*
 * See header for info.
 */
void vHOS_OneWireTemperatureSensor_init(xHOS_OneWireTemperatureSensor_t* pxHandle)
{
	/*	Initialize mutex	*/
	pxHandle->xMutex = xSemaphoreCreateMutexStatic(pxHandle->xMutexMem);
}

/*
 * See header for info.
 */
uint8_t ucHOS_OneWireTemperatureSensor_lock(
	xHOS_OneWireTemperatureSensor_t* pxHandle,
	TickType_t xTimeout	)
{
	return xSemaphoreTake(pxHandle->xMutex, xTimeout);
}

/*
 * See header for info.
 */
void vHOS_OneWireTemperatureSensor_unlock(xHOS_OneWireTemperatureSensor_t* pxHandle)
{
	xSemaphoreGive(pxHandle->xMutex);
}

/*
 * See header for info.
 */
uint8_t ucHOS_OneWireTemperatureSensor_getTemperature(
	xHOS_OneWireTemperatureSensor_t* pxHandle,
	TickType_t xTimeout,
	int32_t* piTemperature	)
{


	return 1;
}




















