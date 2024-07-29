/*
 * OExtendShiftRegister.h
 *
 *  Created on: Feb 29, 2024
 *      Author: Ali Emad
 */

#ifndef COTS_OS_INC_HAL_IOEXTEND_OEXTENDSHIFTREGISTER_H_
#define COTS_OS_INC_HAL_IOEXTEND_OEXTENDSHIFTREGISTER_H_

/*
 * This driver connects shift register to SPI module.
 *
 * This driver is to be "extern" in "port_DIO", to achieve seamless output
 * port interface.
 */

#include "FreeRTOS.h"
#include "semphr.h"

typedef struct{
	/*	PUBLIC	*/
	/*
	 * If HW SPI is used, write unit number to "ucSpiUnitNumber". Otherwise,
	 * write "ucSpiUnitNumber" with value of 255.
	 *
	 */
	uint8_t ucSpiUnitNumber;

	/*	Used only when no HW SPI unit is used	*/
	uint8_t ucSckPort;
	uint8_t ucSckPin;
	uint8_t ucDataPort;
	uint8_t ucDataPin;

	uint8_t ucRegisterSizeInBytes; // maximum: 4-bytes

	uint8_t ucLatchPort;
	uint8_t ucLatchPin;

	/*	Private (May only be used by this driver and lower layer DIO driver	*/
	uint32_t uiCurrentOutputWord;

	SemaphoreHandle_t xMutex;
	StaticSemaphore_t xMutexStatic;
}xHOS_OExtendShiftRegister_t;

/*
 * Initializes handle.
 *
 * Notes:
 * 		-	All public variables of handle must be initially set.
 *
 * 		-	SPI unit must be initialized first.
 */
void vHOS_OExtendShiftRegister_init(xHOS_OExtendShiftRegister_t* pxHandle);

void vHOS_OExtendShiftRegister_writePin(
		xHOS_OExtendShiftRegister_t* pxHandle,
		uint8_t ucPinNumber,
		uint8_t ucVal	);

void vHOS_OExtendShiftRegister_writePort(
		xHOS_OExtendShiftRegister_t* pxHandle,
		uint32_t uiMask,
		uint32_t uiVal	);











#endif /* COTS_OS_INC_HAL_IOEXTEND_OEXTENDSHIFTREGISTER_H_ */
