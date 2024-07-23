/*
 * MAX6675.c
 *
 *  Created on: Jul 21, 2024
 *      Author: Ali Emad
 */



/*	RTOS	*/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "RTOS_PRI_Config.h"

/*	LIB	*/
#include <stdint.h>

/*	MCAL (Ported)	*/
#include "MCAL_Port/Port_DIO.h"

/*	HAL	*/
#include "HAL/SPI/SPI.h"

/*	SELF	*/
#include "HAL/MAX6675/MAX6675.h"



/*******************************************************************************
 * Helping structures:
 ******************************************************************************/
typedef struct{
	uint16_t usReserved0	: 3;
	uint16_t usTempBy4		: 12;
	uint16_t usReserved1	: 1;
}Frame_t;


/*******************************************************************************
 * API functions:
 ******************************************************************************/
void vHOS_MAX6675_init(xHOS_MAX6675_t* pxHandle)
{
	vPort_DIO_initPinOutput(pxHandle->ucCsPort, pxHandle->ucCsPin);
	vPORT_DIO_WRITE_PIN(pxHandle->ucCsPort, pxHandle->ucCsPin, 1);
}

int32_t iHOS_MAX6675_getTemperature(xHOS_MAX6675_t* pxHandle)
{
	/*	Lock SPI unit	*/
	ucHOS_SPI_takeMutex(pxHandle->ucSpiUnitNumber, portMAX_DELAY);

	/*	Select sensor	*/
	vPORT_DIO_WRITE_PIN(pxHandle->ucCsPort, pxHandle->ucCsPin, 0);

	/*	Read 2 bytes	*/
	Frame_t xFrame;
	vHOS_SPI_receive(pxHandle->ucSpiUnitNumber, (int8_t*)&xFrame, 2);

	/*	De-select sensor	*/
	vPORT_DIO_WRITE_PIN(pxHandle->ucCsPort, pxHandle->ucCsPin, 1);

	/*	Unlock SPI unit	*/
	vHOS_SPI_releaseMutex(pxHandle->ucSpiUnitNumber);

	return ((int32_t)xFrame.usTempBy4 * 1000) / 4;
}

















