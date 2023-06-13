/*
 * Port_AFIO.h
 *
 *  Created on: Jun 13, 2023
 *      Author: Ali Emad
 */

#ifndef HAL_OS_PORT_PORT_AFIO_H_
#define HAL_OS_PORT_PORT_AFIO_H_

/*
 * In most of ARM-based MCUs, peripherals are not directly mapped to a constant
 * set ofIO pins. Instead, another peripheral (Alternate Function IO) configures
 * the mapping.
 *
 * If the ported target does not have one, mapping functions are left empty.
 */

static inline void vPort_AFIO_mapSpi(uint8_t ucUnitNumber, uint8_t ucMapNumber)
{

}



#endif /* HAL_OS_PORT_PORT_AFIO_H_ */
