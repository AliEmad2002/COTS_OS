/*
 * MAX6675.h
 *
 *  Created on: Jul 21, 2024
 *      Author: Ali Emad
 */

#ifndef COTS_OS_INC_HAL_MAX6675_MAX6675_H_
#define COTS_OS_INC_HAL_MAX6675_MAX6675_H_

typedef struct{
	uint8_t ucSpiUnitNumber;
	uint8_t ucCsPin;
	uint8_t ucCsPort;
}xHOS_MAX6675_t;

void vHOS_MAX6675_init(xHOS_MAX6675_t* pxHandle);

/*	Returns temperature in milli-degree C	*/
int32_t iHOS_MAX6675_getTemperature(xHOS_MAX6675_t* pxHandle);



#endif /* COTS_OS_INC_HAL_MAX6675_MAX6675_H_ */
