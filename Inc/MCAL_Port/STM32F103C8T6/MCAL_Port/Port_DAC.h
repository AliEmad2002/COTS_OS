/*
 * Port_DAC.h
 *
 *  Created on: Sep 25, 2023
 *      Author: Ali Emad
 */

#ifndef COTS_OS_INC_MCAL_PORT_PORT_DAC_H_
#define COTS_OS_INC_MCAL_PORT_PORT_DAC_H_

/*
 * This driver is ported for the STM32F103C8T6. As it has no DAC HW, the driver
 * will depend on HW generated very fast PWM signals, passed to LPFs, in order
 * to achieve the DAC functionality.
 *
 * Each timer configured for this driver can not be used elsewhere in the same
 * application, and each timer can provide up to 4 channels. Thus, driver can
 * provide a total of 16 DAC channels if given all 4-available timers.
 *
 * In order to generate high frequency, noise-free analog signals on the output
 * of an LPF, its input driving PWM signal must be of a much faster frequency. This
 * driver uses a PWM frequency of about 1MHz. (This should be taken into consideration
 * when designing a PCB!)
 *
 * High frequency PWM would mostly be of high transient spikes due to load mis-match,
 * to overcome this issue, connect a small capacitor between PWM pin and GND, also
 * the capacitor has to be as close as possible to the pin.
 *
 * When tested, the following configuration resulted in a 10kHz noise-free analog
 * signal, which is acceptable for many applications:
 * 		-	Input PWM signal of 1MHz.
 * 		-	Matching capacitor of 220pF.
 * 		-	Simple RC LPF, where R = 1k-Ohms, C = 10nF.
 *
 * If higher speed analog signals were needed, PWM frequency could be increased. But
 * keep in mind, that duty cycle resolution (By extension: resulting voltage resolution)
 * would decrease as the PWM frequency increases.
 *
 * Each DAC unit uses a HW timer unit, such that DAC unit 0 uses Timer unit 0, and
 * so on.
 *
 * Also, each DAC channel uses a HW timer channel, such that DAC channel 0 uses
 * timer channel 0, and so on.
 */

#include "MCAL_Port/Port_Timer.h"

/*******************************************************************************
 * Configuration:
 ******************************************************************************/
#define uiCONF_DAC_PWM_FREQ_HZ			1000000
#define iCONF_DAC_MIN_VOLTAGE_MV		0
#define iCONF_DAC_MAX_VOLTAGE_MV		3300

/*******************************************************************************
 * API functions / macros:
 ******************************************************************************/
/*
 * Initializes DAC unit.
 */
void vPort_DAC_initUnit(uint8_t ucUnitNumber);

/*
 * Initializes DAC channel.
 *
 * Notes:
 * 		-	Pin mapping must be done independently.
 */
void vPort_DAC_initChannel(uint8_t ucUnitNumber, uint8_t ucChannelNumber);

/*
 * Sets channel voltage.
 */
void vPort_DAC_setChannelVoltage(	uint8_t ucUnitNumber,
									uint8_t ucChannelNumber,
									int32_t iVoltageMV	);

/*
 * Gets DAC resolution.
 *
 * This macro evaluates to the number of steps that could be achieved between 0% and
 * 100% of full analog swing on any DAC channel on the given DAC unit.
 */
#define uiPORT_DAC_GET_RESOLUTION(ucUnitNumber)		\
	(	uiPORT_TIM_GET_PWM_RESOLUTION(ucUnitNumber)	)








#endif /* COTS_OS_INC_MCAL_PORT_PORT_DAC_H_ */
