/*
 * SPIConfig.h
 *
 *  Created on: Jun 12, 2023
 *      Author: Ali Emad
 */

#ifndef HAL_OS_INC_SPI_SPICONFIG_H_
#define HAL_OS_INC_SPI_SPICONFIG_H_

#define configHOS_SPI_EN					1

#if configHOS_SPI_EN
#define configHOS_SPI_NUMBER_OF_UNITS		1

/*
 * Differences between SPI driver versions are described here:
 *
 * User may un-comment only one version definition of the following.
 */
#define configHOS_SPI_V1_1
//#define configHOS_SPI_V1_2

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


/***************
 * As the single SPI unit may be used by multiple other drivers (for example connecting
 * TFT and SD-card on the same bus(unit), SPI hardware configuration is done once
 * in this config file.
 **************/
/*	TODO: this part is not completed yet, it is barley working!	*/
#define configHOS_SPI_0_ENABLE_FULL_DUPLEX			1
#define configHOS_SPI_0_SET_FRAME_FORMAT_8_BIT		1
#define configHOS_SPI_0_SET_FRAME_FORMAT_16_BIT		0
#define configHOS_SPI_0_SET_DIRECTION_LSBIT_FIRST	0
#define configHOS_SPI_0_SET_DIRECTION_LSBYTE_FIRST	0
#define configHOS_SPI_0_IS_MASTER					1
#define configHOS_SPI_0_ENABLE_MOSI					1
#define configHOS_SPI_0_ENABLE_MISO					0
#define configHOS_SPI_0_ENABLE_NSS					0
#define configHOS_SPI_0_AFIO_MAP_NUMBER				0
#define configHOS_SPI_0_COM_MODE					3
#define configHOS_SPI_0_BAUDRATE_PRESCALER			256



#endif	/*	configHOS_SPI_EN	*/

#endif /* HAL_OS_INC_SPI_SPICONFIG_H_ */
