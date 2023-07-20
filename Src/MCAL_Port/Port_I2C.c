/*
 * Port_I2C.c
 *
 *  Created on: Jul 9, 2023
 *      Author: Ali Emad
 */

#include "stm32f103xb.h"
#include "stm32f1xx_hal.h"

#include "MCAL_Port/Port_I2C.h"

I2C_TypeDef* const pxPortI2cArr[] = {I2C1, I2C2};


void vPort_I2C_initHardware(uint8_t ucUnitNumber, xPort_I2C_HW_Conf_t* pxConf)
{
	/*	Init pins HW	*/
	vPort_AFIO_mapI2C(ucUnitNumber, pxConf->ucAFIOMapNumber);
	vPort_GPIO_initI2CPins(ucUnitNumber, ucUnitNumber);

	/*	Disable unit before initialization	*/
	vPort_I2C_disable(ucUnitNumber);

	/*	General call setting	*/
	if (pxConf->ucEnableGeneralCall)
		vPort_I2C_enableGeneralCall(ucUnitNumber);
	else
		vPort_I2C_disableGeneralCall(ucUnitNumber);

	/*	Clock stretching setting	*/
	if (pxConf->ucEnableClockStretching)
		vPort_I2C_enableClockStretching(ucUnitNumber);
	else
		vPort_I2C_disableClockStretching(ucUnitNumber);

	/*	Self address setting	*/
	vPort_I2C_writeOwnSlaveAddress(ucUnitNumber, pxConf->usSelfAddress, pxConf->ucIsAddress7Bit);

	/*	Clock mode and frequency setting	*/
	vPort_I2C_setClockModeAndFreq(ucUnitNumber, pxConf->ucClockMode, pxConf->uiSclFrequencyHz);

	/*	Max rising time setting	*/
	vPort_I2C_setMaxRisingTime(ucUnitNumber, pxConf->uiMaxRisingTimeNs);
}
