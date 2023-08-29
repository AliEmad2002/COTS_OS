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
#include "LIB/Graphics/Color.h"
#include "HAL/SPI/SPI.h"


/*******************************************************************************
 * Structures:
 ******************************************************************************/
typedef struct{
	/*		PRIVATE		*/
	/*	Initialization done semaphore	*/
	StaticSemaphore_t xInitDoneSemaphoreStatic;
	SemaphoreHandle_t xInitDoneSemaphore;

	/*		PUBLIC		*/
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

	/*	Dimensions of the display.	*/
	uint32_t uiWidth;
	uint32_t uiHeight;

	/*	Mutex (Must be locked by the task which is using the TFT handle).	*/
	StaticSemaphore_t xMutexStatic;
	SemaphoreHandle_t xMutex;
}xHOS_TFT_t;



/*******************************************************************************
 * API functions / macros:
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
 * Sets value of "xHOS_TFT_Boundaries_t" handle.
 */
#define vHOS_TFT_SET_BOUNDS(xBounds, usStart, usEnd)	       \
{                                                              \
	(xBounds).usStart = xPORT_ROTATE_RIGHT(usStart, 8);		   \
	(xBounds).usEnd = xPORT_ROTATE_RIGHT(usEnd, 8);            \
}


/*
 * Initializes A0, reset and CS pins HW.
 * Executes TFT Initialization commands.
 *
 * Notes:
 * 		-	This functions can only be called in a task after scheduler has started.
 */
void vHOS_TFT_init(xHOS_TFT_t* pxTFT);

/*
 * Blocks the calling task until TFT handle initialization (which is executed by
 * another task) is done.
 */
uint8_t ucHOS_TFT_blockUntilInitDone(xHOS_TFT_t* pxTFT, TickType_t xTimeout);

/*
 * Enables communication with TFT. (selects TFT object to listen to SPI bus Via CS pin)
 *
 * Notes:
 * 		-	Communication enable/disable is controlled using the CS pin.
 *
 * 		-	This function locks the SPI unit, the next function releases it.
 *
 * 		-	Communication is initially enabled in the function "vHOS_TFT_init()".
 *
 * 		-	Communication must be enabled first before calling any of the following
 * 			functions.
 *
 * 		-	Communication may be disabled in application task if the SPI bus is
 * 			going to communicate with other slaves
 */
uint8_t ucHOS_TFT_enableCommunication(xHOS_TFT_t* pxTFT, TickType_t xTimeout);

/*
 * See description of previous function.
 */
void vHOS_TFT_disableCommunication(xHOS_TFT_t* pxTFT);

/*
 * Sets color of a pixel given its coordinates.
 */
void vHOS_TFT_setPix(	xHOS_TFT_t* pxTFT,
						uint16_t usX, uint16_t usY,
						xLIB_Color16_t xColor	);

/*
 * Fills a rectangle with a single color, given its boundaries.
 */
void vHOS_TFT_fillRectangle(	xHOS_TFT_t* pxTFT,
								uint16_t usXStart,
								uint16_t usXEnd,
								uint16_t usYStart,
								uint16_t usYEnd,
								xLIB_Color16_t xColor	);

/*
 * Fills a rectangle with an array of colors, given its boundaries.
 */
void vHOS_TFT_drawRectangle(	xHOS_TFT_t* pxTFT,
								uint16_t usXStart,
								uint16_t usXEnd,
								uint16_t usYStart,
								uint16_t usYEnd,
								xLIB_Color16_t* xColorArr	);

/**********************
 * Some use cases may not be in a need of setting boundaries on every iteration.
 * Instead, boundaries are initially set and filled iteratively per application needs.
 *
 * For these use cases, the following functions would be more time efficient than
 * the above ones.
 *
 * User must keep in mind that these following functions do not perform any range
 * checking (to minimize execution time).
 ***********************/

/*
 * Sets boundaries of x-axis.
 */
void vHOS_TFT_setXBoundaries(xHOS_TFT_t* pxTFT, uint16_t usStart, uint16_t usEnd);

/*
 * Sets boundaries of y-axis.
 */
void vHOS_TFT_setYBoundaries(xHOS_TFT_t* pxTFT, uint16_t usStart, uint16_t usEnd);

/*
 * sets color of the pixel which coordinates are TFT's internal  memory current
 * coordinates
 */
void vHOS_TFT_drawNextPix(xHOS_TFT_t* pxTFT, xLIB_Color16_t xColor);

/*
 * same as the previous function, except it set color of N pixels in row, from an
 * array of colors
 */
void vHOS_TFT_drawNextNPixFromArr(	xHOS_TFT_t* pxTFT,
									xLIB_Color16_t* xColorArr,
									uint32_t uiN	);

/*
 * same as the previous function, except it set color of N pixels in row, by single
 * constant color.
 */
void vHOS_TFT_drawNextNPixFromSingleColor(	xHOS_TFT_t* pxTFT,
											xLIB_Color16_t xColor,
											uint32_t uiN	);













#endif /* HAL_OS_INC_TFT_TFT_H_ */
