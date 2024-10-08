/*
 * TFT.c
 *
 *  Created on: Jun 14, 2023
 *      Author: Ali Emad
 */

/*	LIB	*/
#include <stdint.h>
#include "LIB/Graphics/font8x8.h"

/*	FreeRTOS	*/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/*	MCAL (Ported)	*/
#include "MCAL_Port/Port_ASM.h"
#include "MCAL_Port/Port_DIO.h"

/*	SELF	*/
#include "HAL/TFT/TFT.h"

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
	(vPORT_DIO_WRITE_PIN((pxTFT)->ucCSPort, (pxTFT)->ucCSPin, 0))

#define vTFT_CS_DISABLE(pxTFT)	\
	(vPORT_DIO_WRITE_PIN((pxTFT)->ucCSPort, (pxTFT)->ucCSPin, 1))

/*
 * resets TFT (executes reset sequence).
 */
static void vHOS_TFT_reset(xHOS_TFT_t* pxTFT)
{
	vTaskDelay(pdMS_TO_TICKS(1));

	vPORT_DIO_WRITE_PIN(pxTFT->ucRstPort, pxTFT->ucRstPin, 1);
	vTaskDelay(pdMS_TO_TICKS(1));

	vPORT_DIO_WRITE_PIN(pxTFT->ucRstPort, pxTFT->ucRstPin, 0);
	vTaskDelay(pdMS_TO_TICKS(1));

	vPORT_DIO_WRITE_PIN(pxTFT->ucRstPort, pxTFT->ucRstPin, 1);
	vTaskDelay(pdMS_TO_TICKS(1));

	vPORT_DIO_WRITE_PIN(pxTFT->ucRstPort, pxTFT->ucRstPin, 0);
	vTaskDelay(pdMS_TO_TICKS(1));

	vPORT_DIO_WRITE_PIN(pxTFT->ucRstPort, pxTFT->ucRstPin, 1);
	vTaskDelay(pdMS_TO_TICKS(170));
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
__attribute__((always_inline)) inline
static inline void vHOS_TFT_writeCmd(xHOS_TFT_t* pxTFT, uint8_t ucCmd)
{
#if ucHOS_TFT_DATA_CONNECTION == ucHOS_TFT_DATA_CONNECTION_SPI
	ucHOS_SPI_blockUntilTransferComplete(pxTFT->ucSpiUnitNumber, portMAX_DELAY);
	vPORT_DIO_WRITE_PIN(pxTFT->ucA0Port, pxTFT->ucA0Pin, 0);
	vHOS_SPI_send(pxTFT->ucSpiUnitNumber, (int8_t*)&ucCmd, 1);
#else
	vPORT_DIO_WRITE_PIN(pxTFT->ucWrPort, pxTFT->ucWrPin, 0);
	vPORT_DIO_WRITE_PIN(pxTFT->ucA0Port, pxTFT->ucA0Pin, 0);
	vPORT_DIO_WRITE_PORT(pxTFT->ucDataPort, 0b11111111, ucCmd);
	vPORT_DIO_WRITE_PIN(pxTFT->ucWrPort, pxTFT->ucWrPin, 1);
#endif		/*	ucHOS_TFT_DATA_CONNECTION		*/
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
__attribute__((always_inline)) inline
static inline void vHOS_TFT_writeDataByte(xHOS_TFT_t* pxTFT, int8_t ucByte)
{
#if ucHOS_TFT_DATA_CONNECTION == ucHOS_TFT_DATA_CONNECTION_SPI
	ucHOS_SPI_blockUntilTransferComplete(pxTFT->ucSpiUnitNumber, portMAX_DELAY);
	vPORT_DIO_WRITE_PIN(pxTFT->ucA0Port, pxTFT->ucA0Pin, 1);
	vHOS_SPI_send(pxTFT->ucSpiUnitNumber, (int8_t*)&ucByte, 1);
#else
	vPORT_DIO_WRITE_PIN(pxTFT->ucWrPort, pxTFT->ucWrPin, 0);
	vPORT_DIO_WRITE_PIN(pxTFT->ucA0Port, pxTFT->ucA0Pin, 1);
	vPORT_DIO_WRITE_PORT(pxTFT->ucDataPort, 0b11111111, ucByte);
	vPORT_DIO_WRITE_PIN(pxTFT->ucWrPort, pxTFT->ucWrPin, 1);
#endif		/*	ucHOS_TFT_DATA_CONNECTION		*/
}

static void vHOS_TFT_writeDataArr(xHOS_TFT_t* pxTFT, int8_t* pcArr, uint32_t uiSize)
{
#if ucHOS_TFT_DATA_CONNECTION == ucHOS_TFT_DATA_CONNECTION_SPI
	ucHOS_SPI_blockUntilTransferComplete(pxTFT->ucSpiUnitNumber, portMAX_DELAY);
	vPORT_DIO_WRITE_PIN(pxTFT->ucA0Port, pxTFT->ucA0Pin, 1);
	vHOS_SPI_setByteDirection(	pxTFT->ucSpiUnitNumber,
								ucHOS_SPI_BYTE_DIRECTION_LSBYTE_FIRST	);
	vHOS_SPI_send(pxTFT->ucSpiUnitNumber, pcArr, uiSize);
#else
	for (uint32_t i = 0; i < uiSize; i++)
		vHOS_TFT_writeDataByte(pxTFT, pcArr[i]);
#endif		/*	ucHOS_TFT_DATA_CONNECTION		*/
}

static void vHOS_TFT_writeDataArrMultiple(xHOS_TFT_t* pxTFT, int8_t* pcArr, uint32_t uiSize, uint32_t uiN)
{
#if ucHOS_TFT_DATA_CONNECTION == ucHOS_TFT_DATA_CONNECTION_SPI
	ucHOS_SPI_blockUntilTransferComplete(pxTFT->ucSpiUnitNumber, portMAX_DELAY);
	vPORT_DIO_WRITE_PIN(pxTFT->ucA0Port, pxTFT->ucA0Pin, 1);
	vHOS_SPI_setByteDirection(	pxTFT->ucSpiUnitNumber,
								ucHOS_SPI_BYTE_DIRECTION_LSBYTE_FIRST	);
	vHOS_SPI_sendMultiple(pxTFT->ucSpiUnitNumber, pcArr, uiSize, uiN);
#else
	for (uint32_t i = 0; i < uiN; i++)
		vHOS_TFT_writeDataArr(pxTFT, pcArr, uiSize);
#endif		/*	ucHOS_TFT_DATA_CONNECTION		*/
}

static void vReset(xHOS_TFT_t* pxTFT)
{
	/*	Flush shift register	*/
	vPORT_DIO_WRITE_PORT(pxTFT->ucDataPort, (uint16_t)0xFFFF, 0);
	vPORT_DIO_WRITE_PORT(pxTFT->ucDataPort, (uint16_t)0xFFFF, 0);
	vPORT_DIO_WRITE_PORT(pxTFT->ucDataPort, (uint16_t)0xFFFF, 0);

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

/*******************************************************************************
 * API functions
 ******************************************************************************/
/*
 * See header file for info.
 */
void vHOS_TFT_init(xHOS_TFT_t* pxTFT)
{
	static uint8_t ucIsFirstTime = 1;

	if (ucIsFirstTime)
	{
		ucIsFirstTime = 0;

		/*	Initialize A0, reset, WR and CS pins HW	*/
		vPort_DIO_initPinOutput(pxTFT->ucA0Port, pxTFT->ucA0Pin);
		vPort_DIO_initPinOutput(pxTFT->ucRstPort, pxTFT->ucRstPin);

#if ucHOS_TFT_DATA_CONNECTION == ucHOS_TFT_DATA_CONNECTION_PARALLEL
		vPort_DIO_initPinOutput(pxTFT->ucWrPort, pxTFT->ucWrPin);
		/*	WR is initially low	*/
		vPORT_DIO_WRITE_PIN(pxTFT->ucWrPort, pxTFT->ucWrPin, 0);
#endif		/*	ucHOS_TFT_DATA_CONNECTION		*/

#if ucHOS_TFT_USE_CS == 1
		vPort_DIO_initPinOutput(pxTFT->ucCSPort, pxTFT->ucCSPin);
		/*	CS is initially low (TFT is selected by default)	*/
		vTFT_CS_ENABLE(pxTFT);
#endif		/*	ucHOS_TFT_USE_CS		*/

		/*	initialize TFT mutex	*/
		pxTFT->xMutex = xSemaphoreCreateMutexStatic(&pxTFT->xMutexStatic);
		xSemaphoreGive(pxTFT->xMutex);

		/*	initialize initialization done semaphore	*/
		pxTFT->xInitDoneSemaphore =
			xSemaphoreCreateMutexStatic(&pxTFT->xInitDoneSemaphoreStatic);
		xSemaphoreTake(pxTFT->xInitDoneSemaphore, 0);
	}

	/*	Lock LCD	*/

	/*	reset	*/
	vReset(pxTFT);

	/*	Release initialization done semaphore	*/
	xSemaphoreGive(pxTFT->xInitDoneSemaphore);
}

/*
 * See header file for info.
 */
uint8_t ucHOS_TFT_blockUntilInitDone(xHOS_TFT_t* pxTFT, TickType_t xTimeout)
{
	uint8_t ucDone = xSemaphoreTake(pxTFT->xInitDoneSemaphore, xTimeout);

	if (ucDone == 1)
	{
		xSemaphoreGive(pxTFT->xInitDoneSemaphore);
		return 1;
	}

	else
		return 0;
}

/*
 * See header file for info.
 */
uint8_t ucHOS_TFT_enableCommunication(xHOS_TFT_t* pxTFT, TickType_t xTimeout)
{
#if ucHOS_TFT_DATA_CONNECTION == ucHOS_TFT_DATA_CONNECTION_SPI
	uint8_t ucState = ucHOS_SPI_takeMutex(pxTFT->ucSpiUnitNumber, xTimeout);

	if (ucState)
	{
#if ucHOS_TFT_USE_CS == 1
		vTFT_CS_ENABLE(pxTFT);
#endif		/*	ucHOS_TFT_USE_CS		*/
		return 1;
	}
	else
		return 0;
#else
#if ucHOS_TFT_USE_CS == 1
	vTFT_CS_ENABLE(pxTFT);
#endif		/*	ucHOS_TFT_USE_CS		*/
	return 1;
#endif		/*	ucHOS_TFT_DATA_CONNECTION		*/
}

/*
 * See header file for info.
 */
void vHOS_TFT_disableCommunication(xHOS_TFT_t* pxTFT)
{
#if ucHOS_TFT_USE_CS == 1
	vTFT_CS_DISABLE(pxTFT);
#endif		/*	ucHOS_TFT_USE_CS		*/

#if ucHOS_TFT_DATA_CONNECTION == ucHOS_TFT_DATA_CONNECTION_SPI
	vHOS_SPI_releaseMutex(pxTFT->ucSpiUnitNumber);
#endif		/*	ucHOS_TFT_DATA_CONNECTION		*/
}

/*
 * See header file for info.
 */
void vHOS_TFT_setPix(	xHOS_TFT_t* pxTFT,
						uint16_t usX, uint16_t usY,
						xLIB_Color16_t xColor	)
{
	/*	set boundaries to this pixel	*/
	vHOS_TFT_setXBoundaries(pxTFT, usX, usX);
	vHOS_TFT_setYBoundaries(pxTFT, usY, usY);

	/*	send color data	*/
	vHOS_TFT_writeDataArr(pxTFT, (int8_t*)&xColor, 2);
}

/*
 * See header file for info.
 */
void vHOS_TFT_fillRectangle(	xHOS_TFT_t* pxTFT,
								uint16_t usXStart,
								uint16_t usXEnd,
								uint16_t usYStart,
								uint16_t usYEnd,
								xLIB_Color16_t xColor	)
{
	/*	set boundaries to this rectangle	*/
	vHOS_TFT_setXBoundaries(pxTFT, usXStart, usXEnd);
	vHOS_TFT_setYBoundaries(pxTFT, usYStart, usYEnd);

	/*	send color data	*/
	uint32_t uiN = (usXEnd - usXStart + 1) * (usYEnd - usYStart + 1);
	vHOS_TFT_writeDataArrMultiple(pxTFT, (int8_t*)&xColor, 2, uiN);
}

/*
 * See header file for info.
 */
void vHOS_TFT_drawRectangle(	xHOS_TFT_t* pxTFT,
								uint16_t usXStart,
								uint16_t usXEnd,
								uint16_t usYStart,
								uint16_t usYEnd,
								xLIB_Color16_t* xColorArr	)
{
	/*	set boundaries to this rectangle	*/
	vHOS_TFT_setXBoundaries(pxTFT, usXStart, usXEnd);
	vHOS_TFT_setYBoundaries(pxTFT, usYStart, usYEnd);

	/*	send color data	*/
	uint32_t uiN = (usXEnd - usXStart + 1) * (usYEnd - usYStart + 1);
	vHOS_TFT_writeDataArr(pxTFT, (int8_t*)xColorArr, 2*uiN);
}

/*
 * See header file for info.
 */
void vHOS_TFT_setXBoundaries(xHOS_TFT_t* pxTFT, uint16_t usStart, uint16_t usEnd)
{
	/*	send set x boundaries command	*/
	vHOS_TFT_writeCmd(pxTFT, ucTFT_CMD_SET_X_BOUNDARIES);

	/*	Prepare boundaries 4-bytes	*/
	uint32_t uiBounds =	((uint32_t)usStart) | (((uint32_t)usEnd) << 16);
	uiBounds = uiPort_ASM_reverseEach16(uiBounds);

	/*	send x boundaries in data mode	*/
	vHOS_TFT_writeDataArr(pxTFT, (int8_t*)&uiBounds, 4);

	/*	send color writing command	*/
	vHOS_TFT_writeCmd(pxTFT, ucTFT_CMD_MEM_WRITE);
}

/*
 * See header file for info.
 */
void vHOS_TFT_setYBoundaries(xHOS_TFT_t* pxTFT, uint16_t usStart, uint16_t usEnd)
{
	/*	Prepare boundaries 4-bytes	*/
	uint32_t uiBounds =	((uint32_t)usStart) | (((uint32_t)usEnd) << 16);
	uiBounds = uiPort_ASM_reverseEach16(uiBounds);

	/*	send set y boundaries command	*/
	vHOS_TFT_writeCmd(pxTFT, ucTFT_CMD_SET_Y_BOUNDARIES);

	/*	send x boundaries in data mode	*/
	vHOS_TFT_writeDataArr(pxTFT, (int8_t*)&uiBounds, 4);

	/*	send color writing command	*/
	vHOS_TFT_writeCmd(pxTFT, ucTFT_CMD_MEM_WRITE);
}

/*
 * See header file for info.
 */
__attribute__((always_inline)) inline
void inline vHOS_TFT_drawNextPix(xHOS_TFT_t* pxTFT, xLIB_Color16_t xColor)
{
	/*	send color data	*/
	vHOS_TFT_writeDataArr(pxTFT, (int8_t*)&xColor, 2);
}

/*
 * See header file for info.
 */
__attribute__((always_inline)) inline
void inline vHOS_TFT_drawNextNPixFromArr(	xHOS_TFT_t* pxTFT,
									xLIB_Color16_t* xColorArr,
									uint32_t uiN	)
{
	vHOS_TFT_writeDataArr(pxTFT, (int8_t*)xColorArr, 2*uiN);
}

/*
 * See header file for info.
 */
__attribute__((always_inline)) inline
void inline vHOS_TFT_drawNextNPixFromSingleColor(	xHOS_TFT_t* pxTFT,
											xLIB_Color16_t xColor,
											uint32_t uiN	)
{
	vHOS_TFT_writeDataArrMultiple(pxTFT, (int8_t*)&xColor, 2, uiN);
}

/*
 * See header file for info.
 */
void vHOS_TFT_drawText(	xHOS_TFT_t* pxTFT,
						xLIB_Color16_t xColor,
						uint16_t usXStart,
						uint16_t usYStart,
						uint8_t ucSize,
						char* pcTxt	)
{
	ucSize--;

	for (uint8_t i = 0; pcTxt[i] != '\0'; i++)
	{
		for (uint16_t j = 0; j < 8; j++)
		{
			for (uint16_t k = 0; k < 8; k++)
			{
				if (pucFont8x8[(uint8_t)pcTxt[i]][7-j] & (1 << (k)))
				{
					if (ucSize == 0)
					{
						vHOS_TFT_setPix(pxTFT, usXStart+j, usYStart+k+i*8, xColor);
					}
					else
					{
						vHOS_TFT_fillRectangle(
								pxTFT,

								usXStart + j*ucSize,
								usXStart + (j+1)*ucSize,

								usYStart + i*ucSize*8 + k*ucSize,
								usYStart + i*ucSize*8 + (k+1)*ucSize,

								xColor
							);
					}
				}
			}
		}
	}
}














