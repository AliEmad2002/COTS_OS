/*
 * Port_Flash.h
 *
 *  Created on: Mar 3, 2024
 *      Author: Ali Emad
 */

/*	Target checking	*/
#include "MCAL_Port/Port_Target.h"
#ifdef MCAL_PORT_TARGET_STM32F401RCT6

#ifndef COTS_OS_INC_MCAL_PORT_STM32F401RCT6_MCAL_PORT_PORT_FLASH_H_
#define COTS_OS_INC_MCAL_PORT_STM32F401RCT6_MCAL_PORT_PORT_FLASH_H_

/*
 * RAM temporary buffer:
 * 		-	When modifying a word in flash memory, the scenario that's mostly
 * 			used is storing temporary copy of the whole sector containing this
 * 			word, modifying, and finally re-writing to flash.
 *
 * 		-	This operation requires a RAM buffer that's at least equal in size to
 * 			flash sector.
 *
 * 		-	As there may be multiple tasks / drivers modifying flash in runtime,
 * 			a single shared buffer is better be used.
 *
 * 		-	This shared buffer is defined and allocated only when the following
 * 			macro is uncommented.
 *
 * 		-	Higher layer code must consider size of this buffer when accessing.
 * 			It is defined by: "uiPORT_FLASH_SHARED_RAM_BUFFER_SIZE_IN_BYTES".
 */
#define ucPORT_FLASH_USE_SHARED_RAM_BUFFER

#ifdef ucPORT_FLASH_USE_SHARED_RAM_BUFFER
	#define uiPORT_FLASH_SHARED_RAM_BUFFER_SIZE_IN_BYTES	(16 * 1024)

	extern uint8_t pucPortFlashRamTemporaryBuffer[uiPORT_FLASH_SHARED_RAM_BUFFER_SIZE_IN_BYTES];
#endif


/*	Initialize flash interface peripheral	*/
void vPort_Flash_init(void);

/*	Unlock flash memory	*/
void vPort_Flash_unlock(void);

/*	Lock flash memory	*/
void vPort_Flash_lock(void);

/*
 * Given a memory bound (starting address and size in bytes), this function
 * returns index of flash sector at which this memory bound starts, and number of
 * sectors across which it extends.
 *
 * Notes:
 * 		-	If the given memory boundaries are not within flash boundaries, function
 * 			writes -1 to "puiStartSector".
 */
void vPort_Flash_getSectors(	void* pvStart,
								uint32_t uiSizeInBytes,
								int32_t* piStartSector,
								uint32_t* puiNumberOfSectors	);

/*
 * Given a memory bound (index of starting sector and number of sectors),
 * this function returns address of flash location at which this memory bound
 * starts, and size in bytes of this bound.
 *
 * Notes:
 * 		-	If the given memory boundaries are not within flash boundaries, function
 * 			writes 0xFFFFFFFF to "ppvStart".
 */
void vPort_Flash_getAddress(	uint32_t uiStartSector,
								uint32_t uiNumberOfSectors,
								void** ppvStart,
								uint32_t* puiSizeInBytes	);

/*
 * Erases a sector defined flash memory section.
 */
void vPort_Flash_erase(uint32_t uiStartSector, uint32_t uiNumberOfSectors);

#ifdef ucPORT_FLASH_USE_SHARED_RAM_BUFFER

	/*
	 * Locks RAM temporary buffer of the flash driver.
	 */
	uint8_t ucPort_Flash_lockSharedRamBuffer(TickType_t xTimeout);

	/*
	 * Unlocks RAM temporary buffer of the flash driver.
	 */
	void vPort_Flash_unlockSharedRamBuffer(void);

#endif	/*	ucPORT_FLASH_USE_SHARED_RAM_BUFFER	*/

/*
 * Writes data to an initially erased section in flash memory.
 *
 * Notes:
 * 		-	Flash memory must be initially unlocked.
 */
void vPort_Flash_write(void* pvDst, uint8_t* pucData, uint32_t uiSizeInBytes);


#endif /* COTS_OS_INC_MCAL_PORT_STM32F401RCT6_MCAL_PORT_PORT_FLASH_H_ */


#endif	/*	Target cheking	*/
