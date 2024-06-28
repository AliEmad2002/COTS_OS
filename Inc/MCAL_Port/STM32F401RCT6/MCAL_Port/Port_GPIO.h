/*
 * Port_GPIO.h
 *
 *  Created on: Jun 13, 2023
 *      Author: Ali Emad
 */

/*	Target checking	*/
#include "MCAL_Port/Port_Target.h"
#ifdef MCAL_PORT_TARGET_STM32F401RCT6


#ifndef HAL_OS_PORT_PORT_GPIO_H_
#define HAL_OS_PORT_PORT_GPIO_H_

/*
 * This file is used to initialize GPIO pins associated with other peripherals.
 */


/*
 * Initializes SPI associated pins.
 *
 * Notes:
 * 		-	Last three arguments are set when their pins are to be initialized,
 * 		and reset otherwise.
 */
void vPort_GPIO_initSpiPins(	uint8_t ucUnitNumber,
								uint8_t ucInitNSS,
								uint8_t ucInitMISO,
								uint8_t ucInitMOSI	);

/*
 * Initializes UART associated pins.
 */
void vPort_GPIO_initUartPins(	uint8_t ucUnitNumber,
								uint8_t ucInitTx,
								uint8_t ucInitRx	);

/*
 * De-initializes UART associated pins.
 */
void vPort_GPIO_deInitUartPins(	uint8_t ucUnitNumber,
								uint8_t ucDeInitTx,
								uint8_t ucDeInitRx	);

/*
 * Initializes I2C associated pins.
 */
void vPort_GPIO_initI2cPins(uint8_t ucUnitNumber);

/*
 * Initializes timer associated pins.
 *
 * Notes:
 */
void vPort_GPIO_initTimChannelPin(	uint8_t ucUnitNumber,
									uint8_t ucChannelNumber	);

/*
 * Initializes ADC channel associated pin as analog input.
 */
void vPort_GPIO_initAdcChannelPinAsInput(	uint8_t ucAdcUnitNumber,
											uint8_t ucAdcChannelNumber	);

#endif /* HAL_OS_PORT_PORT_GPIO_H_ */


#endif /* Target checking */
