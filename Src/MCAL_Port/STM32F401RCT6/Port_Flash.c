/*
 * Port_Flash.c
 *
 *  Created on: Mar 11, 2024
 *      Author: Ali Emad
 */

/*	Target checking	*/
#include "MCAL_Port/Port_Target.h"
#ifdef MCAL_PORT_TARGET_STM32F401RCT6


/*	LIB	*/
#include <stdint.h>
#include "LIB/Assert.h"

/*	RTOS	*/
#include "FreeRTOS.h"
#include "semphr.h"

/*	MCAL	*/
#include "stm32f4xx.h"

/*	SELF	*/
#include "MCAL_Port/Port_Flash.h"


/*******************************************************************************
 * Drivers global variables:
 ******************************************************************************/
/*	RAM temporary buffer.	*/
#ifdef ucPORT_FLASH_USE_SHARED_RAM_BUFFER
	uint8_t pucPortFlashRamTemporaryBuffer[uiPORT_FLASH_SHARED_RAM_BUFFER_SIZE_IN_BYTES];
#endif

/*******************************************************************************
 * Drivers private variables:
 ******************************************************************************/
/*	Mutex of RAM temporary buffer.	*/
#ifdef ucPORT_FLASH_USE_SHARED_RAM_BUFFER
	static SemaphoreHandle_t xRamTemporaryBufferMutex;
	static StaticSemaphore_t xRamTemporaryBufferMutexStatic;
#endif

const uint32_t puiSectorStartAddressArr[] = {
		0x08000000,
		0x08004000,
		0x08008000,
		0x0800C000,
		0x08010000,
		0x08020000,
		0x08040000
};

/*******************************************************************************
 * Private functions:
 ******************************************************************************/
/*
 * Returns number of sector containing the given address.
 *
 * Notes:
 * 		-	If given address is not within the flash memory, function returns -1.
 */
static int32_t iGetSectorNumber(void* pvAddress)
{
	/*	Check that address is within flash boundaries	*/
	if ((uint32_t)pvAddress < 0x08000000 || (uint32_t)pvAddress > 0x0803FFFF)
		return -1;

	/*	Check for sectors of odd size (Sectors 4 and 5)	*/
	if ((uint32_t)pvAddress >= 0x08010000 && (uint32_t)pvAddress <= 0x0801FFFF)
		return 4;

	if ((uint32_t)pvAddress >= 0x08020000 && (uint32_t)pvAddress <= 0x0803FFFF)
		return 5;

	/*	Sectors of regular size	*/
	return ((uint32_t)pvAddress - 0x08000000) / (16 * 1024);
}

/*******************************************************************************
 * API functions:
 ******************************************************************************/
/*
 * See header for info.
 */
void vPort_Flash_init(void)
{
	/*	Create mutex of RAM temporary buffer (if used)	*/
	#ifdef ucPORT_FLASH_USE_SHARED_RAM_BUFFER
		xRamTemporaryBufferMutex =
				xSemaphoreCreateMutexStatic(&xRamTemporaryBufferMutexStatic);
	#endif
}

/*
 * See header for info.
 */
inline void vPort_Flash_unlock(void)
{
	HAL_FLASH_Unlock();
}

/*
 * See header for info.
 */
inline void vPort_Flash_lock(void)
{
	HAL_FLASH_Lock();
}

/*
 * See header for info.
 */
void vPort_Flash_getSectors(	void* pvStart,
								uint32_t uiSizeInBytes,
								int32_t* piStartSector,
								uint32_t* puiNumberOfSectors	)
{
	/*	Get start sector	*/
	int32_t iStartSector = iGetSectorNumber(pvStart);

	if (iStartSector == -1)
		*piStartSector = -1;

	*piStartSector = iStartSector;

	/*	Get end sector	*/
	int32_t iEndSector = iGetSectorNumber((void*)((uint32_t)pvStart + uiSizeInBytes));
	if (iEndSector == -1)
		*piStartSector = -1;

	/*	Get number of sectors	*/
	*puiNumberOfSectors = iEndSector - iStartSector + 1;
}

/*
 * See header for info.
 */
void vPort_Flash_getAddress(	uint32_t uiStartSector,
								uint32_t uiNumberOfSectors,
								void** ppvStart,
								uint32_t* puiSizeInBytes	)
{
	/*	Get number of ending sector	*/
	uint32_t uiAfterEndSector = uiStartSector + uiNumberOfSectors;

	/*	Check boundaries	*/
	if (uiStartSector > 5 || uiAfterEndSector > 6)
		*ppvStart = (void*)0xFFFFFFFF;

	/*	Get starting address	*/
	*ppvStart = (void*)(puiSectorStartAddressArr[uiStartSector]);

	/*	Get ending address	*/
	void* pvEnd = (void*)(puiSectorStartAddressArr[uiAfterEndSector]);

	/*	Get size in bytes	*/
	*puiSizeInBytes = (uint32_t)pvEnd - (uint32_t)(*ppvStart);
}

/*
 * See header for info.
 */
void vPort_Flash_erase(uint32_t uiStartSector, uint32_t uiNumberOfSectors)
{
	FLASH_EraseInitTypeDef FlashErase;
	uint32_t SectorError = 0;

	FlashErase.Sector = uiStartSector;
	FlashErase.NbSectors = uiNumberOfSectors;
	FlashErase.TypeErase = FLASH_TYPEERASE_SECTORS;
	FlashErase.VoltageRange = VOLTAGE_RANGE_3;

	do{
		HAL_FLASHEx_Erase(&FlashErase, &SectorError);
	}while(SectorError != 0xffffffff);
}

#ifdef ucPORT_FLASH_USE_SHARED_RAM_BUFFER

	/*
	 * See header for info.
	 */
	uint8_t ucPort_Flash_lockSharedRamBuffer(TickType_t xTimeout)
	{
		return xSemaphoreTake(xRamTemporaryBufferMutex, xTimeout);
	}

	/*
	 * See header for info.
	 */
	void vPort_Flash_unlockSharedRamBuffer(void)
	{
		xSemaphoreGive(xRamTemporaryBufferMutex);
	}

#endif	/*	ucPORT_FLASH_USE_SHARED_RAM_BUFFER	*/

/*
 * See header for info.
 */
void vPort_Flash_write(void* pvDst, uint8_t* pucData, uint32_t uiSizeInBytes)
{
	/*	Check boundaries	*/
	if ((uint32_t)pvDst < puiSectorStartAddressArr[0] || (uint32_t)pvDst > puiSectorStartAddressArr[6])
		vLib_ASSERT(0, 0);

	for(uint32_t i = 0; i < uiSizeInBytes; i++)
	{
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, (uint32_t)pvDst + i, pucData[i]);
	}
}


#endif	/*	Target cheking	*/
