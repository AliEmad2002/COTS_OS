/*
 * TFT.c
 *
 *  Created on: Jun 14, 2023
 *      Author: Ali Emad
 */

/*	LIB	*/
#include <stdint.h>
#include "Std_Types.h"
#include "Bit_Math.h"

/*	FreeRTOS	*/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/*	MCAL (Ported)	*/
#include "Port/Port_DIO.h"
#include "Port/Port_AFIO.h"
#include "Port/Port_GPIO.h"
#include "Port/Port_SPI.h"
#include "Port/Port_Interrupt.h"

/*	SELF	*/
#include "Inc/TFT/TFT.h"

/*******************************************************************************
 * Helping functions/macros:
 ******************************************************************************/
#define ucTFT_CMD_SLEEP_OUT								0x11
#define ucTFT_CMD_INTERFACE_PIXEL_FORMAT				0x3A
#define ucTFT_CMD_DISPLAY_ON							0x29
#define ucTFT_CMD_SET_X_BOUNDARIES						0x2A
#define ucTFT_CMD_SET_Y_BOUNDARIES						0x2B
#define ucTFT_CMD_MEM_WRITE								0x2C
#define ucTFT_CMD_SCROLL								0x37

#define vTFT_CS_ENABLE(pxTFT)	\
	(vPort_DIO_writePin((pxTFT)->ucCSPort, (pxTFT)->ucCSPin, 0))

#define vTFT_CS_DISABLE(pxTFT)	\
	(vPort_DIO_writePin((pxTFT)->ucCSPort, (pxTFT)->ucCSPin, 1))

/*
 * resets TFT (executes reset sequence).
 */
static inline void vHOS_TFT_reset(xHOS_TFT_t* pxTFT)
{
	vTaskDelay(pdMS_TO_TICKS(1));

	vPort_DIO_writePin(pxTFT->ucRstPort, pxTFT->ucRstPin, 1);
	vTaskDelay(pdMS_TO_TICKS(1));

	vPort_DIO_writePin(pxTFT->ucRstPort, pxTFT->ucRstPin, 0);
	vTaskDelay(pdMS_TO_TICKS(1));

	vPort_DIO_writePin(pxTFT->ucRstPort, pxTFT->ucRstPin, 1);
	vTaskDelay(pdMS_TO_TICKS(1));

	vPort_DIO_writePin(pxTFT->ucRstPort, pxTFT->ucRstPin, 0);
	vTaskDelay(pdMS_TO_TICKS(1));

	vPort_DIO_writePin(pxTFT->ucRstPort, pxTFT->ucRstPin, 1);
	vTaskDelay(pdMS_TO_TICKS(120));
}

/*
 * Enters CMD mode, then sends a command.
 *
 * Notes:
 * 		-	CMD mode is entered by resetting A0 pin.
 * 		-	But first, SPI should've completed any pending send commands, which
 * 			can be achieved by acquiring the SPI_Transfer_Mutex.
 * 		-	This function is not ISR safe!
 *
 */
static inline void vHOS_TFT_writeCmd(xHOS_TFT_t* pxTFT, uint8_t ucCmd)
{
	SemaphoreHandle_t xTransferMutex = xHOS_SPI_getTransferMutexHandle(pxTFT->ucSpiUnitNumber);
	xSemaphoreTake(xTransferMutex, portMAX_DELAY);

	vPort_DIO_writePin(pxTFT->ucA0Port, pxTFT->ucA0Pin, 0);
	vHOS_SPI_send(pxTFT->ucSpiUnitNumber, (int8_t*)&ucCmd, 1);
}

/*
 * Enters Data mode, then sends a byte.
 *
 * Notes:
 * 		-	Data mode is entered by setting A0 pin.
 * 		-	But first, SPI should've completed any pending send commands, which
 * 			can be achieved by acquiring the SPI_Transfer_Mutex.
 * 		-	This function is not ISR safe!
 *
 */
static inline void vHOS_TFT_writeDataByte(xHOS_TFT_t* pxTFT, int8_t ucByte)
{
	SemaphoreHandle_t xTransferMutex = xHOS_SPI_getTransferMutexHandle(pxTFT->ucSpiUnitNumber);
	xSemaphoreTake(xTransferMutex, portMAX_DELAY);

	vPort_DIO_writePin(pxTFT->ucA0Port, pxTFT->ucA0Pin, 1);
	vHOS_SPI_send(pxTFT->ucSpiUnitNumber, (int8_t*)&ucByte, 1);
}

static inline void vHOS_TFT_writeDataArr(xHOS_TFT_t* pxTFT, int8_t* pcArr, uint32_t uiSize)
{
	SemaphoreHandle_t xTransferMutex = xHOS_SPI_getTransferMutexHandle(pxTFT->ucSpiUnitNumber);
	xSemaphoreTake(xTransferMutex, portMAX_DELAY);

	vPort_DIO_writePin(pxTFT->ucA0Port, pxTFT->ucA0Pin, 1);
	vHOS_SPI_send(pxTFT->ucSpiUnitNumber, pcArr, uiSize);
}

static inline void vHOS_TFT_writeDataArrMultiple(xHOS_TFT_t* pxTFT, int8_t* pcArr, uint32_t uiSize, uint32_t uiN)
{
	SemaphoreHandle_t xTransferMutex = xHOS_SPI_getTransferMutexHandle(pxTFT->ucSpiUnitNumber);
	xSemaphoreTake(xTransferMutex, portMAX_DELAY);

	vPort_DIO_writePin(pxTFT->ucA0Port, pxTFT->ucA0Pin, 1);
	vHOS_SPI_sendMultiple(pxTFT->ucSpiUnitNumber, pcArr, uiSize, uiN);
}

/*******************************************************************************
 * API functions
 ******************************************************************************/
/*
 * See header file for info.
 */
void vHOS_TFT_init(xHOS_TFT_t* pxTFT)
{
	/*	Initializes A0, reset and CS pins HW	*/
	vPort_DIO_initPinOutput(pxTFT->ucA0Port, pxTFT->ucA0Pin);
	vPort_DIO_initPinOutput(pxTFT->ucRstPort, pxTFT->ucRstPin);
	vPort_DIO_initPinOutput(pxTFT->ucCSPort, pxTFT->ucCSPin);

	/*	CS is initially low (TFT is selected by default)	*/
	vTFT_CS_ENABLE(pxTFT);

	/*	initialize TFT mutex	*/
	pxTFT->xMutexHandle = xSemaphoreCreateBinaryStatic(&pxTFT->xMutexMemory);
	configASSERT(pxTFT->xMutexHandle != NULL);
	xSemaphoreGive(pxTFT->xMutexHandle);

	/*	execute reset sequence	*/
	vHOS_TFT_reset(pxTFT);

	/*	sleep out command	*/
	vHOS_TFT_writeCmd(pxTFT, ucTFT_CMD_SLEEP_OUT);
	vHOS_TFT_writeCmd(pxTFT, ucTFT_CMD_SLEEP_OUT);

	/*	set color mode to RGB565	*/
	vHOS_TFT_writeCmd(pxTFT, ucTFT_CMD_INTERFACE_PIXEL_FORMAT);
	vHOS_TFT_writeDataByte(pxTFT, 0x05);

	/*	display on	*/
	vHOS_TFT_writeCmd(pxTFT, ucTFT_CMD_DISPLAY_ON);
}

/*
 * See header file for info.
 */
inline __attribute__((always_inline))
void vHOS_TFT_enableCommunication(xHOS_TFT_t* pxTFT)
{
	vTFT_CS_ENABLE(pxTFT);
}

/*
 * See header file for info.
 */
inline __attribute__((always_inline))
void vHOS_TFT_disableCommunication(xHOS_TFT_t* pxTFT)
{
	vTFT_CS_DISABLE(pxTFT);
}

/*
 * See header file for info.
 */
void vHOS_TFT_setPix(	xHOS_TFT_t* pxTFT,
						uint16_t usX, uint16_t usY,
						uint16_t usColor	)
{
	/*	set boundaries to this pixel	*/
	xHOS_TFT_Boundaries_t xBounds = {usY, usY, usX, usX};
	vHOS_TFT_setBoundaries(pxTFT, &xBounds);

	/*	send color data	*/
	vHOS_TFT_writeDataArr(pxTFT, (int8_t*)&usColor, 2);
}

/*
 * See header file for info.
 */
void vHOS_TFT_fillRectangle(	xHOS_TFT_t* pxTFT,
								xHOS_TFT_Boundaries_t* pxBounds,
								uint16_t usColor	)
{
	/*	set boundaries to this rectangle	*/
	vHOS_TFT_setBoundaries(pxTFT, pxBounds);

	/*	send color data	*/
	uint32_t uiN = (pxBounds->usX1 - pxBounds->usX0 + 1) * (pxBounds->usY1 - pxBounds->usY0 + 1);
	vHOS_TFT_writeDataArrMultiple(pxTFT, (int8_t*)&usColor, 2, uiN);
}

/*
 * See header file for info.
 */
void vHOS_TFT_drawRectangle(	xHOS_TFT_t* pxTFT,
								xHOS_TFT_Boundaries_t* pxBounds,
								uint16_t* pusColorArr	)
{
	/*	set boundaries to this rectangle	*/
	vHOS_TFT_setBoundaries(pxTFT, pxBounds);

	/*	send color data	*/
	uint32_t uiN = (pxBounds->usX1 - pxBounds->usX0 + 1) * (pxBounds->usY1 - pxBounds->usY0 + 1);
	vHOS_TFT_writeDataArr(pxTFT, (int8_t*)pusColorArr, 2*uiN);
}

/*
 * See header file for info.
 */
void vHOS_TFT_setBoundaries(xHOS_TFT_t* pxTFT, xHOS_TFT_Boundaries_t* pxBounds)
{
	/*	send set y boundaries command	*/
	vHOS_TFT_writeCmd(pxTFT, ucTFT_CMD_SET_Y_BOUNDARIES);

	/*	send y boundaries in data mode	*/
	vHOS_TFT_writeDataArr(pxTFT, ((int8_t*)pxBounds), 4);

	/*	send set x boundaries command	*/
	vHOS_TFT_writeCmd(pxTFT, ucTFT_CMD_SET_X_BOUNDARIES);

	/*	send x boundaries in data mode	*/
	vHOS_TFT_writeDataArr(pxTFT, ((int8_t*)pxBounds) + 4, 4);

	/*	send color writing command	*/
	vHOS_TFT_writeCmd(pxTFT, ucTFT_CMD_MEM_WRITE);
}

/*
 * See header file for info.
 */
void vHOS_TFT_drawNextPix(xHOS_TFT_t* pxTFT, uint16_t usColor)
{
	/*	send color data	*/
	vHOS_TFT_writeDataArr(pxTFT, (int8_t*)&usColor, 2);
}

/*
 * See header file for info.
 */
void vHOS_TFT_drawNextNPixFromArr(xHOS_TFT_t* pxTFT, uint16_t* pusColorArr, uint32_t uiN)
{
	vHOS_TFT_writeDataArr(pxTFT, (int8_t*)pusColorArr, 2*uiN);
}

/*
 * See header file for info.
 */
void vHOS_TFT_drawNextNPixFromSingleColor(xHOS_TFT_t* pxTFT, uint16_t usColor, uint32_t uiN)
{
	vHOS_TFT_writeDataArrMultiple(pxTFT, (int8_t*)&usColor, 2, uiN);
}

