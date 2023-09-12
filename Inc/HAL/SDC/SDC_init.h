/*
 * SDC_init.h
 *
 *  Created on: Aug 17, 2023
 *      Author: Ali Emad
 *
 * Notes:
 * 		-	This file is private and could only be used by SDC driver.
 *
 *		-	For all of this file's functions which interact with the SDC handle,
 *			handle's mutex must be first taken the calling function, and must be
 *			released right after it's been of no need.
 *
 *		-	For all of this file's functions which use SPI (or call functions
 *			which use SPI themselves), SPI mutex must be taken first, and released
 *			as soon as SPI is no longer needed.
 */

#ifndef COTS_OS_INC_HAL_SDC_SDC_INIT_H_
#define COTS_OS_INC_HAL_SDC_SDC_INIT_H_


/*
 * Following are the branches labeled on the initialization flow diagram, each
 * branch has a timeout of 1 second. And the flow itself.
 *
 * Notes:
 * 		-	Diagram is at the directory: ../Inc/HAL/SDC.
 * 		-	These functions must be called after running the scheduler.
 */
xHOS_SDC_Version_t xHOS_SDC_initBranch2(xHOS_SDC_t* pxSdc);
xHOS_SDC_Version_t xHOS_SDC_initBranch1(xHOS_SDC_t* pxSdc);
xHOS_SDC_Version_t xHOS_SDC_initBranch0(xHOS_SDC_t* pxSdc);
uint8_t ucHOS_SDC_initFlow(xHOS_SDC_t* pxSdc);







#endif /* COTS_OS_INC_HAL_SDC_SDC_INIT_H_ */
