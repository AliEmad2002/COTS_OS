/*
 * SPI_V1.0.c
 *
 *  Created on: Jun 17, 2023
 *      Author: Ali Emad
 *
 *
 * Notes:
 * 		-	Delayed! read SPI_V1.1 document.
 */

/*	LIB	*/
#include <stdint.h>
#include "Std_Types.h"
#include "Bit_Math.h"

/*	FreeRTOS	*/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/*	MCAL (Ported)	*/
#include "Port/Port_SPI.h"
#include "Port/Port_Interrupt.h"
#include "Port/Port_GPIO.h"
#include "Port/Port_AFIO.h"

/*	SELF	*/
#include "Inc/SPI/SPI.h"

#if configHOS_SPI_EN
#ifdef configHOS_SPI_V1_0


/*******************************************************************************
 * Global and static variables:
 ******************************************************************************/
/*
 * Mutexes:
 *
 * Notes:
 * 		-	"pxSPIUnitMutexArr[i]" and "SpiTransferMutexArr[i]" are described in
 * 			the header file.
 *
 * 		-	"pxSPIHWMutexArr[i]" is taken by SPI driver task before sending a byte,
 * 			and released by ISR. This is done to assure that SPI driver task does
 * 			not transfer another byte while the previous one has not yet ended.
 */
static StaticSemaphore_t pxSPIHWStaticMutexArr[configHOS_SPI_NUMBER_OF_UNITS];
static StaticSemaphore_t pxSPIUnitStaticMutexArr[configHOS_SPI_NUMBER_OF_UNITS];
static StaticSemaphore_t pxSPITransferStaticMutexArr[configHOS_SPI_NUMBER_OF_UNITS];

SemaphoreHandle_t pxSPIHWMutexArr[configHOS_SPI_NUMBER_OF_UNITS];
SemaphoreHandle_t pxSPIUnitMutexArr[configHOS_SPI_NUMBER_OF_UNITS];
SemaphoreHandle_t pxSPITransferMutexArr[configHOS_SPI_NUMBER_OF_UNITS];

/*
 * Byte direction setting array.
 *
 * Notes:
 * 		-	"pucSPIByteDirectionArr[i]" is configured using the interface API.
 * 		-	Based on the value of "pucSPIByteDirectionArr[i]", SPI driver task
 * 			decides what end of the array to start transferring from.
 */
uint8_t pucSPIByteDirectionArr[configHOS_SPI_NUMBER_OF_UNITS];

/*******************************************************************************
 * Helping functions/macros:
 ******************************************************************************/
#define uiHOS_SPI_GET_INDEX_OF_NEXT_BYTE(ucUnitNumber, uiCount, uiSize)                  \
(                                                                                        \
	(pucSPIByteDirectionArr[(ucUnitNumber)] == ucHOS_SPI_BYTE_DIRECTION_LSBYTE_FIRST) ?  \
		(uiCount) : ((uiSize) - (uiCount) - 1)                                           \
)

#define vHOS_SPI_SEND_NEXT_BYTE(ucUnitNumber, pcArr, uiCount, uiSize)                          \
{                                                                                              \
	uint32_t uiIndex = uiHOS_SPI_GET_INDEX_OF_NEXT_BYTE((ucUnitNumber), (uiCount), (uiSize));  \
	(uiCount)++;                                                                               \
	vPort_SPI_WRT_DR_NO_WAIT((ucUnitNumber), (pcArr)[uiIndex]);	                               \
}

/*******************************************************************************
 * Driver initialization:
 ******************************************************************************/
/*
 * Notes:
 * 		-	This function is externed and called in "HAL_OS.c".
 *		-	HW settings like frame format and clock mode must be initially set
 * 		by user.
 */
BaseType_t xHOS_SPI_init(void)
{
	for (uint8_t i = 0; i < configHOS_SPI_NUMBER_OF_UNITS; i++)
	{
		/*	create HW mutex	*/
		pxSPIHWMutexArr[i] = xSemaphoreCreateBinaryStatic(&pxSPIHWStaticMutexArr[i]);
		configASSERT(pxSPIHWMutexArr[i] != NULL);
		xSemaphoreGive(pxSPIHWMutexArr[i]);

		/*	create unit mutex	*/
		pxSPIUnitMutexArr[i] = xSemaphoreCreateBinaryStatic(&pxSPIUnitStaticMutexArr[i]);
		configASSERT(pxSPIUnitMutexArr[i] != NULL);
		xSemaphoreGive(pxSPIUnitMutexArr[i]);

		/*	create transfer mutex	*/
		pxSPITransferMutexArr[i] = xSemaphoreCreateBinaryStatic(&pxSPITransferStaticMutexArr[i]);
		configASSERT(pxSPITransferMutexArr[i] != NULL);
		xSemaphoreGive(pxSPITransferMutexArr[i]);

		/*
		 * Enable TC interrupt in the interrupt controller, and set its priority
		 * to the maximum possible (i.e.: configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY).
		 * TODO: is this the best priority to choose?
		 */
		vPort_Interrupt_setPriority(pxPortInterruptSpiIrqNumberArr[i], configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
		vPort_Interrupt_enableIRQ(pxPortInterruptSpiIrqNumberArr[i]);

		/*
		 * Initially enable TC interrupt from SPI HW itself
		 */
		vPORT_SPI_ENABLE_TXC_INTERRUPT(i);
	}

	return pdPASS;
}

/*******************************************************************************
 * API functions:
 ******************************************************************************/
void vHOS_SPI_setByteDirection(uint8_t ucUnitNumber, uint8_t ucByteDirection)
{
	pucSPIByteDirectionArr[ucUnitNumber] = ucByteDirection;
}

void vHOS_SPI_send(uint8_t ucUnitNumber, int8_t* pcArr, uint32_t uiSize)
{
	uint32_t uiCount = 0;

	/*	send	*/
	while(uiCount < uiSize)
	{
		xSemaphoreTake(pxSPIHWMutexArr[ucUnitNumber], 0);
		vHOS_SPI_SEND_NEXT_BYTE(ucUnitNumber, pcArr, uiCount, uiSize);
		xSemaphoreTake(pxSPIHWMutexArr[ucUnitNumber], portMAX_DELAY);
	}

	xSemaphoreGive(pxSPITransferMutexArr[ucUnitNumber]);
}

void vHOS_SPI_transceive(uint8_t ucUnitNumber, int8_t* pcOutArr, int8_t* pcInArr, uint32_t uiSize)
{
	uint32_t uiCount = 0;
	uint32_t uiIndex;

	/*	transceive	*/
	while(uiCount < uiSize)
	{
		xSemaphoreTake(pxSPIHWMutexArr[ucUnitNumber], 0);
		uiIndex = uiHOS_SPI_GET_INDEX_OF_NEXT_BYTE(ucUnitNumber, uiCount, uiSize);
		vPort_SPI_WRT_DR_NO_WAIT(ucUnitNumber, pcOutArr[uiIndex]);
		xSemaphoreTake(pxSPIHWMutexArr[ucUnitNumber], portMAX_DELAY);
		pcInArr[uiIndex] = vPort_SPI_GET_DR_NO_WAIT(ucUnitNumber);
		uiCount++;
	}

	xSemaphoreGive(pxSPITransferMutexArr[ucUnitNumber]);
}

void vHOS_SPI_sendMultiple(uint8_t ucUnitNumber, int8_t* pcArr, uint32_t uiSize, uint32_t uiN)
{
	uint32_t uiCount;

	while(uiN--)
	{
		uiCount = 0;

		/*	send	*/
		while(uiCount < uiSize)
		{
			xSemaphoreTake(pxSPIHWMutexArr[ucUnitNumber], 0);
			vHOS_SPI_SEND_NEXT_BYTE(ucUnitNumber, pcArr, uiCount, uiSize);
			xSemaphoreTake(pxSPIHWMutexArr[ucUnitNumber], portMAX_DELAY);
		}
	}

	xSemaphoreGive(pxSPITransferMutexArr[ucUnitNumber]);
}

inline __attribute__((always_inline))
SemaphoreHandle_t xHOS_SPI_getUnitMutexHandle(uint8_t ucUnitNumber)
{
	return pxSPIUnitMutexArr[ucUnitNumber];
}

inline __attribute__((always_inline))
SemaphoreHandle_t xHOS_SPI_getTransferMutexHandle(uint8_t ucUnitNumber)
{
	return pxSPITransferMutexArr[ucUnitNumber];
}

/*******************************************************************************
 * ISRs:
 ******************************************************************************/
#define HANDLER(n)                                                      \
void configHOS_SPI_HANDLER_##n (void)                                   \
{                                                                       \
	vPORT_SPI_CLEAR_TXC_FLAG(n);                                        \
                                                                        \
	BaseType_t xHighPriorityTaskWoken = pdFALSE;                        \
	xSemaphoreGiveFromISR(pxSPIHWMutexArr[n], &xHighPriorityTaskWoken); \
	portYIELD_FROM_ISR(xHighPriorityTaskWoken);                         \
}

#if (configHOS_SPI_NUMBER_OF_UNITS > 0)
HANDLER(0)
#endif

#if (configHOS_SPI_NUMBER_OF_UNITS > 1)
HANDLER(1)
#endif

#if (configHOS_SPI_NUMBER_OF_UNITS > 2)
HANDLER(2)
#endif

#if (configHOS_SPI_NUMBER_OF_UNITS > 3)
HANDLER(3)
#endif

#if (configHOS_SPI_NUMBER_OF_UNITS > 4)
HANDLER(4)
#endif

#if (configHOS_SPI_NUMBER_OF_UNITS > 5)
HANDLER(5)
#endif

#if (configHOS_SPI_NUMBER_OF_UNITS > 6)
HANDLER(6)
#endif

#if (configHOS_SPI_NUMBER_OF_UNITS > 7)
HANDLER(7)
#endif

#if (configHOS_SPI_NUMBER_OF_UNITS > 8)
HANDLER(8)
#endif

#if (configHOS_SPI_NUMBER_OF_UNITS > 9)
HANDLER(9)
#endif

/*	To generate the previous few lines, use this code:	*/
/*
#include <stdio.h>

int main() {
    for (int i = 0; i < 10; i++)
    {
        printf("#if (configHOS_SPI_NUMBER_OF_UNITS > %d)\n", i);
        printf("HANDLER(%d)\n", i);
        printf("#endif\n");
        printf("\n");
    }
    return 0;
}
 */


#endif	/*	configHOS_SPI_V1_1	*/
#endif	/*	configHOS_SPI_EN	*/
