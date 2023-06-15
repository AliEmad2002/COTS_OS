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

/*
 * resets TFT (executes reset sequence).
 */
static inline void vHOS_TFT_reset(xHOS_TFT_t* pxTFT)
{
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

static inline void vHOS_TFT_writeDataArr(xHOS_TFT_t* pxTFT, int8_t* pcArr, uint32_t uiN)
{
	SemaphoreHandle_t xTransferMutex = xHOS_SPI_getTransferMutexHandle(pxTFT->ucSpiUnitNumber);
	xSemaphoreTake(xTransferMutex, portMAX_DELAY);

	vPort_DIO_writePin(pxTFT->ucA0Port, pxTFT->ucA0Pin, 1);
	vHOS_SPI_send(pxTFT->ucSpiUnitNumber, pcArr, uiN);
}

/*******************************************************************************
 * API functions
 ******************************************************************************/
/*
 * See header file for info.
 */
void vHOS_TFT_init(xHOS_TFT_t* pxTFT, uint16_t usSpiBaudratePrescaler, uint8_t ucSpiAfioMapNumber)
{
	/*	Initializes A0, reset pins HW	*/
	vPort_DIO_initPinOutput(pxTFT->ucA0Port, pxTFT->ucA0Pin);
	vPort_DIO_initPinOutput(pxTFT->ucRstPort, pxTFT->ucRstPin);

	/*	TODO: initialize TFT mutex	*/

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
								uint16_t usColor	);

/*
 * See header file for info.
 */
void vHOS_TFT_drawRectangle(	xHOS_TFT_t* pxTFT,
								xHOS_TFT_Boundaries_t* pxBounds,
								uint16_t* pusColorArr	);

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
void vHOS_TFT_drawNextNPixFromArr(xHOS_TFT_t* pxTFT, uint16_t* pusColorArr, uint32_t uiN);

/*
 * See header file for info.
 */
void vHOS_TFT_drawNextNPixFromSingleColor(xHOS_TFT_t* pxTFT, uint16_t usColor, uint32_t uiN);
