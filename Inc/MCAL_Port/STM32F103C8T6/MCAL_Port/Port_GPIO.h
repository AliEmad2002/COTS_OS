/*
 * Port_GPIO.h
 *
 *  Created on: Jun 13, 2023
 *      Author: Ali Emad
 */

/*	Target checking	*/
#include "MCAL_Port/Port_Target.h"
#ifdef MCAL_PORT_TARGET_STM32F103C8T6


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

/*
 * Initializes UART associated pins.
 */
void vPort_GPIO_initUartPins(	uint8_t ucUnitNumber,
								uint8_t ucMapNumber,
								uint8_t ucInitTx,
								uint8_t ucInitRx	);

/*
 * Initializes timer channel associated pin as output.
 */
void vPort_GPIO_initTimerChannelPinAsOutput(	uint8_t ucTimerUnitNumber,
												uint8_t ucTimerChannelNumber,
												uint8_t ucMapNumber	);

/*
 * Initializes DAC channel associated pin as output.
 */
void vPort_GPIO_initDacChannelPinAsOutput(	uint8_t ucDacUnitNumber,
											uint8_t ucDacChannelNumber,
											uint8_t ucMapNumber	);

/*
 * Initializes DAC channel associated pin as output.
 */
void vPort_GPIO_initDacChannelPinAsOutput(	uint8_t ucDacUnitNumber,
											uint8_t ucDacChannelNumber,
											uint8_t ucMapNumber	);

/*
 * Initializes ADC channel associated pin as analog input.
 */
void vPort_GPIO_initAdcChannelPinAsOutput(	uint8_t ucAdcUnitNumber,
											uint8_t ucAdcChannelNumber	);

#endif /* HAL_OS_PORT_PORT_GPIO_H_ */


#endif /* Target checking */