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
#define configHOS_SPI_V1_0
//#define configHOS_SPI_V1_1
//#define configHOS_SPI_V1_2


#endif	/*	configHOS_SPI_EN	*/

#endif /* HAL_OS_INC_SPI_SPICONFIG_H_ */