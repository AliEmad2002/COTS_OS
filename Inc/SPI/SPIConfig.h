/*
 * SPIConfig.h
 *
 *  Created on: Jun 12, 2023
 *      Author: Ali Emad
 */

#ifndef HAL_OS_INC_SPI_SPICONFIG_H_
#define HAL_OS_INC_SPI_SPICONFIG_H_

#define configHOS_SPI_NUMBER_OF_UNITS		1

/*
 * Define SPI transfer complete handlers.
 *
 * Notes:
 * 		-	In order for the driver "SPI.c" to be able to use these handlers,
 * 		they must be defined like the following example:
 * 			#define configHOS_SPI_HANDLER_0		SPI1_IRQHandler
 * 			#define configHOS_SPI_HANDLER_1		SPI2_IRQHandler
 * 			... And so on.
 */
#define configHOS_SPI_HANDLER_0		SPI1_IRQHandler

/*
 * 	If target has one SPI handler for multiple events, user must define macros
 * 	for event checking.
 */
#define configHOS_SPI_IS_TC(ucUnitNumber)	\
	(!ucHOS_SPI_IS_BUSY((ucUnitNumber)))




#endif /* HAL_OS_INC_SPI_SPICONFIG_H_ */
