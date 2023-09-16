/*
 * Port_BKP.h
 *
 *  Created on: Sep 15, 2023
 *      Author: Ali Emad
 *
 * This driver controls the low power back-up hardware.
 */

#ifndef COTS_OS_INC_MCAL_PORT_PORT_BKP_H_
#define COTS_OS_INC_MCAL_PORT_PORT_BKP_H_

#include "LIB/Assert.h"

#include "stm32f1xx.h"
#include "stm32f1xx_ll_pwr.h"

/*
 * Initializes back-up peripheral.
 * (Left empty if no initialization needed)
 */
static inline void vPort_BKP_init(void)
{

}

/*
 * Unlocks back-up data registers.
 * (They are normally locked against writing, to avoid parasitic writes)
 */
static inline void vPort_BKP_unlockDataRegisters(void)
{
	LL_PWR_EnableBkUpAccess();
}

/*
 * Locks back-up data registers.
 */
static inline void vPort_BKP_lockDataRegisters(void)
{
	LL_PWR_DisableBkUpAccess();
}

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
static inline void vPort_BKP_writeDataRegister(uint32_t uiRegNum, uint16_t usData)
{
	vLib_ASSERT(uiRegNum < 10, 0);	/*	STM32F103C8T6 has only 10 registers	*/

	(&BKP->DR1)[uiRegNum] = (uint32_t)usData;
}

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
static inline uint16_t usPort_BKP_readDataRegister(uint32_t uiRegNum)
{
	vLib_ASSERT(uiRegNum < 10, 0);	/*	STM32F103C8T6 has only 10 registers	*/

	return (uint16_t)((&BKP->DR1)[uiRegNum] & 0xFFFF);
}













#endif /* COTS_OS_INC_MCAL_PORT_PORT_BKP_H_ */
