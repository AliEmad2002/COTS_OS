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
 * When tested, an input PWM signal of 1MHz could maximumly result in a 1kHz noise-free
 * analog signal, which is acceptable for many applications.
 *
 * If higher speed analog signals were needed, PWM frequency could be increased. But
 * keep in mind, that duty cycle resolution (By extension: resulting voltage resolution)
 * would decrease as the PWM frequency increases.
 */




#endif /* COTS_OS_INC_MCAL_PORT_PORT_DAC_H_ */
