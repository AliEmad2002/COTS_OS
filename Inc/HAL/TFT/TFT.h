/*
 * TFT.h
 *
 *  Created on: Jun 14, 2023
 *      Author: Ali Emad
 *
 * Notes:
 * 		-	For standardization, color format is 16-bit RGB565 format. (Less than
 * 			this would be oppressive for coloring, more is oppressive for memory)
 */

#ifndef HAL_OS_INC_TFT_TFT_H_
#define HAL_OS_INC_TFT_TFT_H_

/*******************************************************************************
 * Include dependencies:
 ******************************************************************************/
#include "HAL/SPI/SPI.h"

/*******************************************************************************
 * Dependencies checks:
 ******************************************************************************/
#if configHOS_SPI_EN == 0
#warning TFT driver can not run without enabling SPI driver!
#endif

/*******************************************************************************
 * Structures:
 ******************************************************************************/
typedef struct{
	/*	number of SPI unit used to communicate the TFT	*/
	uint8_t ucSpiUnitNumber;

	/*	reset pin of the TFT module.	*/
	uint8_t ucRstPin;
	uint8_t ucRstPort;

	/*	A0 pin of the TFT module.	*/
	uint8_t ucA0Pin;
	uint8_t ucA0Port;

	/*	Chip Select pin of the TFT module.	*/
	uint8_t ucCSPin;
	uint8_t ucCSPort;

	/*	Dimensions of the display	*/
	uint32_t uiWidth;
	uint32_t uiHeight;

	/*	Mutex	*/
	StaticSemaphore_t xMutexMemory;	// used *only* internally by the driver for creating mutexHandle
	SemaphoreHandle_t xMutexHandle;
}xHOS_TFT_t;

typedef struct{
	uint16_t usY1;
	uint16_t usY0;

	uint16_t usX1;
	uint16_t usX0;
}xHOS_TFT_Boundaries_t;

typedef struct{
	uint16_t usX;
	uint16_t usY;
}xHOS_TFT_Point_t;
/*******************************************************************************
 * API functions:
 *
 * Notes:
 * 		-	All of the following functions are not ISR safe. (Must not be used
 *	 		in an ISR)
 *
 *	 	-	Application task must first take TFT object's mutex and SPI uint's mutex
 *	 		before drawing.
 *
 *	 	-	Application task must first take SPI unit mutex before drawing. Otherwise
 *	 		TFT driver task may interfere with other tasks which use the same SPI but.
 ******************************************************************************/
/*
 * Initializes A0, reset and CS pins HW.
 * Executes TFT Initialization commands.
 *
 * Notes:
 * 		-	This functions can only be called in a task after scheduler has started.
 */
void vHOS_TFT_init(xHOS_TFT_t* pxTFT);

/*
 * Enables communication with TFT. (selects TFT object to listen to SPI bus Via CS pin)
 *
 * Notes:
 * 		-	Communication is initially enabled in the function "vHOS_TFT_init()".
 *
 * 		-	Communication must be enabled first before calling any of the following
 * 			functions.
 *
 * 		-	Communication may be disabled in application task if the SPI bus is
 * 			going to communicate with other slaves
 */
void vHOS_TFT_enableCommunication(xHOS_TFT_t* pxTFT);

/*
 * See description of previous function.
 */
void vHOS_TFT_disableCommunication(xHOS_TFT_t* pxTFT);

/*
 * Sets color of a pixel given its coordinates.
 */
void vHOS_TFT_setPix(	xHOS_TFT_t* pxTFT,
						uint16_t usX, uint16_t usY,
						uint16_t usColor	);

/*
 * Fills a rectangle with a single color, given its boundaries.
 */
void vHOS_TFT_fillRectangle(	xHOS_TFT_t* pxTFT,
								xHOS_TFT_Boundaries_t* pxBounds,
								uint16_t usColor	);

/*
 * Fills a rectangle with an array of colors, given its boundaries.
 */
void vHOS_TFT_drawRectangle(	xHOS_TFT_t* pxTFT,
								xHOS_TFT_Boundaries_t* pxBounds,
								uint16_t* pusColorArr	);

/**********************
 * Some use cases may not be in a need of setting boundaries on every iteration.
 * Instead, boundaries are initially set and filled iteratively per application needs.
 *
 * For these use cases, the following functions would be more time efficient than
 * the above ones.
 *
 * User must keep in mind that these following functions do not perform any range
 * checking.
 ***********************/
/*
 * Sets boundaries of drawing.
 */
void vHOS_TFT_setBoundaries(xHOS_TFT_t* pxTFT, xHOS_TFT_Boundaries_t* pxBounds);

/*
 * sets color of the pixel which coordinates are TFT's internal  memory current
 * coordinates
 */
void vHOS_TFT_drawNextPix(xHOS_TFT_t* pxTFT, uint16_t usColor);

/*
 * same as the previous function, except it set color of N pixels in row, from an
 * array of colors
 */
void vHOS_TFT_drawNextNPixFromArr(xHOS_TFT_t* pxTFT, uint16_t* pusColorArr, uint32_t uiN);

/*
 * same as the previous function, except it set color of N pixels in row, by single
 * constant color.
 */
void vHOS_TFT_drawNextNPixFromSingleColor(xHOS_TFT_t* pxTFT, uint16_t usColor, uint32_t uiN);

#endif /* HAL_OS_INC_TFT_TFT_H_ */
