/*
 * SPI.c
 *
 *  Created on: Jun 12, 2023
 *      Author: Ali Emad
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

/*	SELF	*/
#include "Inc/SPI/SPI.h"

#if configHOS_SPI_EN

typedef struct{
	uint8_t* arr;
	uint32_t current;
	uint32_t size;
}xSPI_Buffer_t;

/*******************************************************************************
 * Private configurations:
 ******************************************************************************/
#define uiSPI_STACK_SIZE		configMINIMAL_STACK_SIZE

/*******************************************************************************
 * Global and static variables:
 ******************************************************************************/
static volatile xSPI_Buffer_t pxSPIBufferArr[configHOS_SPI_NUMBER_OF_UNITS];

static StaticSemaphore_t pxSPIHWStaticMutexArr[configHOS_SPI_NUMBER_OF_UNITS];
static StaticSemaphore_t pxSPIStaticMutexArr[configHOS_SPI_NUMBER_OF_UNITS];

SemaphoreHandle_t pxSPIHWMutexArr[configHOS_SPI_NUMBER_OF_UNITS];
SemaphoreHandle_t pxSPIMutexArr[configHOS_SPI_NUMBER_OF_UNITS];

uint8_t pucSPIIsTransferComplete[configHOS_SPI_NUMBER_OF_UNITS];

static StackType_t puxSPITaskStackArr[configHOS_SPI_NUMBER_OF_UNITS][configMINIMAL_STACK_SIZE];
static StaticTask_t pxSPITaskStaticHandleArr[configHOS_SPI_NUMBER_OF_UNITS];
static TaskHandle_t pxSPITaskHandleArr[configHOS_SPI_NUMBER_OF_UNITS];

static uint8_t pucParamArr[configHOS_SPI_NUMBER_OF_UNITS];

/*******************************************************************************
 * Task function:
 ******************************************************************************/
void vSPIn_Task(void* pvParams)
{
	uint8_t ucUnitNumber = *((int8_t*)pvParams);
	volatile xSPI_Buffer_t* pxBuffer = &pxSPIBufferArr[ucUnitNumber];
	SemaphoreHandle_t* pxHWMutexHandle = &pxSPIHWMutexArr[ucUnitNumber];
	uint8_t* pucIsTransComplete = &pucSPIIsTransferComplete[ucUnitNumber];
	TaskHandle_t* pxTaskHandle = &pxSPITaskHandleArr[ucUnitNumber];

	while(1)
	{
		if (pxBuffer->current == pxBuffer->size)
		{
			*pucIsTransComplete = 1;
			vTaskSuspend(*pxTaskHandle);
			continue;
		}

		else
		{
			xSemaphoreTake(*pxHWMutexHandle, portMAX_DELAY);
			vHOS_SPI_WRT_DR_NO_WAIT(ucUnitNumber, pxBuffer->arr[pxBuffer->current]);
			pxBuffer->current++;
		}
	}
}

/*
 * Initializes the task/s responsible for this  driver.
 * Notes:
 * 		-	This function is externed and called in "HAL_OS.c".
 *		-	HW settings like frame format and clock mode must be initially set
 * 		by user.
 */
BaseType_t xHOS_SPI_initTasks(void)
{
	for (uint8_t i = 0; i < configHOS_SPI_NUMBER_OF_UNITS; i++)
	{
		/*	reset buffer	*/
		pxSPIBufferArr[i].current = 0;
		pxSPIBufferArr[i].size = 0;

		/*	create HW mutex	*/
		pxSPIHWMutexArr[i] = xSemaphoreCreateBinaryStatic(&pxSPIHWStaticMutexArr[i]);
		configASSERT(pxSPIHWMutexArr[i] != NULL);

		/*	create mutex	*/
		pxSPIMutexArr[i] = xSemaphoreCreateBinaryStatic(&pxSPIStaticMutexArr[i]);
		configASSERT(pxSPIMutexArr[i] != NULL);

		/*	initially transfer is said to be completed	*/
		pucSPIIsTransferComplete[i] = 1;

		/*	create task	*/
		pucParamArr[i] = i;
		pxSPITaskHandleArr[i] = xTaskCreateStatic(	vSPIn_Task,
													"",
													uiSPI_STACK_SIZE,
													&pucParamArr[i],
													configNORMAL_TASK_PRIORITY,
													puxSPITaskStackArr[i],
													&pxSPITaskStaticHandleArr[i]	);

		if (pxSPITaskHandleArr[i] == NULL)
			return pdFAIL;

		/*	Enable TC interrupt	*/
		vHOS_SPI_enableTransferCompleteInterrupt(i);
	}

	return pdPASS;
}

/*******************************************************************************
 * API functions:
 ******************************************************************************/
void vHOS_SPI_send(uint8_t ucUnitNumber, uint8_t* pucArr, uint32_t uiSize)
{
	pxSPIBufferArr[ucUnitNumber].arr = pucArr;
	pxSPIBufferArr[ucUnitNumber].current = 0;
	pxSPIBufferArr[ucUnitNumber].size = uiSize;

	vTaskResume(pxSPITaskHandleArr[ucUnitNumber]);
}

/*******************************************************************************
 * ISRs:
 ******************************************************************************/
#define HANDLER(n)                                                      \
void configHOS_SPI_HANDLER_##n (void)                                   \
{                                                                       \
	if (!configHOS_SPI_IS_TC(n))                                        \
		return;                                                         \
                                                                        \
	BaseType_t xHighPriorityTaskWoken = pdFALSE;                        \
	xSemaphoreGiveFromISR(pxSPIHWMutexArr[n], &xHighPriorityTaskWoken); \
	portYIELD_FROM_ISR(xHighPriorityTaskWoken);                         \
}                                                                       \

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



#endif	/*	configHOS_SPI_EN	*/
