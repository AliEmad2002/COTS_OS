/*
 * UART_Config.h
 *
 *  Created on: Oct 8, 2023
 *      Author: Ali Emad
 */

#ifndef COTS_OS_INC_HAL_UART_UART_CONFIG_H_
#define COTS_OS_INC_HAL_UART_UART_CONFIG_H_


/*
 * Total number of UART units required for the application.
 */
#define uiCONF_UART_NUMBER_OF_NEEDED_UNITS			3

/*
 * SW units' pins.
 */
static const uint8_t pucCONF_UART_SW_UNIT_RX_PORT_ARR[] = {
	1
};

static const uint8_t pucCONF_UART_SW_UNIT_RX_PIN_ARR[] = {
	9
};

/*
 * SW units' baudrates.
 */
static const uint32_t puiCONF_UART_SW_UNIT_BAUDRATE_ARR[] = {
	9600
};

#endif /* COTS_OS_INC_HAL_UART_UART_CONFIG_H_ */
