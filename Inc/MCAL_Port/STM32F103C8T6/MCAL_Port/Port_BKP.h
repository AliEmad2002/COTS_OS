/*
 * Port_BKP.h
 *
 *  Created on: Sep 15, 2023
 *      Author: Ali Emad
 *
 * This driver controls the low power back-up hardware.
 */

/*	Target checking	*/
#include "MCAL_Port/Port_Target.h"
#ifdef MCAL_PORT_TARGET_STM32F103C8T6


#ifndef COTS_OS_INC_MCAL_PORT_PORT_BKP_H_
#define COTS_OS_INC_MCAL_PORT_PORT_BKP_H_

#include "LIB/Assert.h"

#include "stm32f1xx.h"
#include "stm32f1xx_ll_pwr.h"

/*
 * Initializes back-up peripheral.
 * (Left empty if no initialization needed)
 */
void vPort_BKP_init(void);

/*
 * Unlocks back-up data registers.
 * (They are normally locked against writing, to avoid parasitic writes)
 */
#define vPORT_BKP_UNLOCK_DATA_REGISTERS()	\
	(	LL_PWR_EnableBkUpAccess()	)

/*
 * Locks back-up data registers.
 */
#define vPORT_BKP_LOCK_DATA_REGISTERS()	\
	(	LL_PWR_DisableBkUpAccess()	)

/*
 * Writes data to a certain register.
 *
 * Notes:
 * 		-	This driver assumes data registers are of 16-bit. And the upper layer
 * 			code is based on so. Hence, if the used target is not of the same
 * 			implementation, SW work-around is made in this function.
 *
 * 		-	Data registers must be unlocked first.
 */
#define vPORT_BKP_WRITE_DATA_REGISTER(uiRegNum, usData)	\
	(	(&BKP->DR1)[(uiRegNum)] = (uint32_t)(usData)	)

/*
 * Reads data from a certain register.
 *
 * Notes:
 * 		-	This driver assumes data registers are of 16-bit. And the upper layer
 * 			code is based on so. Hence, if the used target is not of the same
 * 			implementation, SW work-around is made in this function.
 *
 * 		-	Data registers must be unlocked first.
 */
#define usPORT_BKP_READ_DATA_REGISTER(uiRegNum)	\
	(	(uint16_t)((&BKP->DR1)[(uiRegNum)] & 0xFFFF)	)













#endif /* COTS_OS_INC_MCAL_PORT_PORT_BKP_H_ */


#endif /* Target checking */