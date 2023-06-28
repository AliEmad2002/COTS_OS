/*
 * Port_BusyWait1us.h
 *
 *  Created on: Jun 28, 2023
 *      Author: Ali Emad
 */

#ifndef HAL_OS_PORT_PORT_BUSYWAIT1US_H_
#define HAL_OS_PORT_PORT_BUSYWAIT1US_H_

/*
 * In some drivers, a delay time of one microsecond is needed, this time interval
 * in most targets is less than time of a context switch, so it would be more efficient
 * to busy wait a single microsecond rather than blocking on it.
 */

static inline void vPort_BusyWait1us(void)
{
	register uint8_t count = 60;
	while(count--)
		asm volatile ("NOP");
}



#endif /* HAL_OS_PORT_PORT_BUSYWAIT1US_H_ */
