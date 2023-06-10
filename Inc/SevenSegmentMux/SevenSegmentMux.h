/*
 * SevenSegmentMux.h
 *
 *  Created on: Jun 9, 2023
 *      Author: Ali Emad
 */

#ifndef HAL_OS_INC_SEVENSEGMENTMUX_SEVENSEGMENTMUX_H_
#define HAL_OS_INC_SEVENSEGMENTMUX_SEVENSEGMENTMUX_H_

#include "Inc/SevenSegmentMux/SevenSegmentMuxConfig.h"

/*
 * "xHOS_SevenSegmentMux_t" structure.
 *
 * Notes:
 * 		-	"pucSegmentPortPinArr[i]" holds port and pin numbers of i-th segment,
 * 			such that: 0-th segment == 'A' segment, ..., 7-th segment == '.' segment.
 *
 * 		-	"pxDigitEnablePortPinArr[i]" holds port and pin numbers of i-th digit
 * 			enable pin.
 *
 *	 	-	The above arrays are user allocated, defined and then passed
 * 			to "pxHOS_SevenSegmentMux_init()" when creating object.
 *
 * 		-	"pucDisplayBuffer[]" in its i-th nibble holds the number to be displayed
 * 			on the i-th digit.
 *
 * 		-	"ucSegmentActiveLevel" is the level that when applied on a single segment
 * 			it beams on (as long as the containing digit is enabled).
 *
 *		-	"ucEnableActiveLevel" is the level that when applied on an enable pin of
 *			a digit it beams on (as long as its segment pins are active).
 *
 *		-	"ucIsEnabled":
 *				0==> Digits are turned off totally and updates to them are ignored.
 *				1==> Digits are turned active and  periodically updated.
 *
 *		-	"ucNumberOfDigits" is number of digits the object has.
 *
 *		-	"ucCurrentActiveDigit" is number of currently active digit.
 *
 *		-	"cPointIndex" is number of the digit that has the point active. it is
 *			set to -1 when point is not currently active at any digit.
 */
typedef struct{
	/*	Configuration parameters	*/
	xHOS_DIO_t* pxSegmentPortPinArr;

	xHOS_DIO_t* pxDigitEnablePortPinArr;

	uint8_t ucSegmentActiveLevel : 1;

	uint8_t ucEnableActiveLevel : 1;

	uint8_t ucIsEnabled : 1;

	uint8_t ucNumberOfDigits;

	/*	Runtime changing parameters	*/
	uint8_t pucDisplayBuffer[(configHOS_SEVEN_SEGMENT_MUX_MAX_NUMBER_OF_DIGITS+1)/2];

	uint8_t ucCurrentActiveDigit;

	int8_t cPointIndex;
}xHOS_SevenSegmentMux_t;

/*
 * Initializes "xHOS_SevenSegmentMux_t" object.
 *
 * Notes:
 * 		-	All arguments are described previously in "struct xHOS_SevenSegmentMux_t".
 *
 * 		-	Returns pointer to the created handle. Used in the following functions.
 */
xHOS_SevenSegmentMux_t* pxHOS_SevenSegmentMux_init(	xHOS_DIO_t* pxSegmentPortPinArr,
													xHOS_DIO_t* pxDigitEnablePortPinArr,
													uint8_t ucSegmentActiveLevel,
													uint8_t ucEnableActiveLevel,
													uint8_t ucNumberOfDigits	);

/*
 * Changes current display value.
 */
void vHOS_SevenSegmentMux_write(	xHOS_SevenSegmentMux_t* pxSevenSegmentMuxHandle,
									uint32_t uiNum,
									int8_t cPointIndex	);

/*
 * Enables "xHOS_SevenSegmentMux_t" object.
 *
 * Notes:
 * 		-	Objects are initially disabled on creation.
 */
void vHOS_SevenSegmentMux_Enable(xHOS_SevenSegmentMux_t* pxSevenSegmentMuxHandle);

/*
 * Disables "xHOS_SevenSegmentMux_t" object.
 *
 * Notes:
 * 		-	Objects are initially disabled on creation.
 */
void vHOS_SevenSegmentMux_Disable(xHOS_SevenSegmentMux_t* pxSevenSegmentMuxHandle);

#endif /* HAL_OS_INC_SEVENSEGMENTMUX_SEVENSEGMENTMUX_H_ */
