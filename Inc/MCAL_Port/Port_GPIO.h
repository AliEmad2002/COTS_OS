/*
 * Port_GPIO.h
 *
 *  Created on: Jun 13, 2023
 *      Author: Ali Emad
 */

#ifndef HAL_OS_PORT_PORT_GPIO_H_
#define HAL_OS_PORT_PORT_GPIO_H_

/*
 * This file is used to initialize GPIO pins associated with other peripherals.
 *
 * Notes:
 * 		-	These functions won't be available in ST-drivers. As STM32CubeIDE
 * 		does not need them, as pins are initially configured in the GUI with
 * 		static code, not in runtime.
 */

/*
 * Initializes SPI associated pins.
 *
 * Notes:
 * 		-	Last three arguments are set when their pins are to be initialized,
 * 		and reset otherwise.
 */
void vPort_GPIO_initSpiPins(	uint8_t ucUnitNumber,
								uint8_t ucMapNumber,
								uint8_t ucInitNSS,
								uint8_t ucInitMISO,
								uint8_t ucInitMOSI	);

/*	Initializes I2C associated pins.	*/
void vPort_GPIO_initI2CPins(uint8_t ucUnitNumber, uint8_t ucMapNumber);

#endif /* HAL_OS_PORT_PORT_GPIO_H_ */
