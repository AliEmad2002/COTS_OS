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
#include "Port/Port_Interrupt.h"
#include "Port/Port_GPIO.h"
#include "Port/Port_AFIO.h"

/*	SELF	*/
#include "Inc/SPI/SPI.h"

#if configHOS_SPI_EN

typedef struct{
	int8_t* pcArr;
	uint32_t uiCount;
	uint32_t uiSize;
}xSPI_Buffer_t;

/*******************************************************************************
 * Private configurations:
 ******************************************************************************/
#define uiSPI_STACK_SIZE		configMINIMAL_STACK_SIZE

/*******************************************************************************
 * Helping structures:
 ******************************************************************************/
typedef struct{
	uint8_t ucFullDuplexEn     : 1;
	uint8_t ucFrameFormat8     : 1;
	uint8_t ucFrameFormat16    : 1;
	uint8_t ucLSBitFirst       : 1;
	uint8_t ucLSByteFirst      : 1;
	uint8_t ucIsMaster         : 1;
	uint8_t ucMOSIEn           : 1;
	uint8_t ucMISOEn           : 1;
	uint8_t ucNssEn            : 1;
	uint8_t ucAFIOMapNumber;
	uint8_t ucComMode;
	uint16_t usBaudratePrescaler;
}xHOS_SPI_HW_Conf_t;

/*******************************************************************************
 * Global and static variables:
 ******************************************************************************/
/*
 * Array of buffers in which SPI transfer data are temporarily stored.
 */
static volatile xSPI_Buffer_t pxSPIBufferArr[configHOS_SPI_NUMBER_OF_UNITS];

/*
 * Mutexes:
 *
 * Notes:
 * 		-	Difference between "pxSPIUnitMutexArr[i]" and "pxSPITransferMutexArr[i]"
 * 			is mentioned in the header file.
 *
 * 		-	In addition, "pxSPIHWMutexArr[i]" is taken by SPI driver task before sending
 * 			a byte, and released by ISR. This is done to assure that SPI driver task
 * 			does not transfer another byte while the previous one has not yet ended.
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

/*
 * Stack and task handle of SPI driver task/s.
 */
static StackType_t puxSPITaskStackArr[configHOS_SPI_NUMBER_OF_UNITS][configMINIMAL_STACK_SIZE];
static StaticTask_t pxSPITaskStaticHandleArr[configHOS_SPI_NUMBER_OF_UNITS];
static TaskHandle_t pxSPITaskHandleArr[configHOS_SPI_NUMBER_OF_UNITS];

/*
 * Task parameters array.
 *
 * Notes:
 * 		-	As each SPI unit has its own RTOS task, while all tasks are using the
 *	 		same function code, parameter must be passed to the function at task
 *	 		initialization to let the code know to which SPI unit it is related.
 */
static uint8_t pucParamArr[configHOS_SPI_NUMBER_OF_UNITS];

/*******************************************************************************
 * Task function:
 ******************************************************************************/
void vSPIn_Task(void* pvParams)
{
	uint8_t ucUnitNumber = *((int8_t*)pvParams);
	volatile xSPI_Buffer_t* pxBuffer = &pxSPIBufferArr[ucUnitNumber];
	SemaphoreHandle_t* pxHWMutexHandle = &pxSPIHWMutexArr[ucUnitNumber];
	SemaphoreHandle_t* pxTransferMutexHandle = &pxSPITransferMutexArr[ucUnitNumber];
	TaskHandle_t* pxTaskHandle = &pxSPITaskHandleArr[ucUnitNumber];
	uint32_t uiIndex;

	while(1)
	{
		if (pxBuffer->uiCount == pxBuffer->uiSize)
		{
			xSemaphoreGive(*pxTransferMutexHandle);
			vTaskSuspend(*pxTaskHandle);
		}

		else
		{
			xSemaphoreTake(*pxHWMutexHandle, portMAX_DELAY);

			if(pucSPIByteDirectionArr[ucUnitNumber] == ucHOS_SPI_BYTE_DIRECTION_LSBYTE_FIRST)
				uiIndex = pxBuffer->uiCount;
			else
				uiIndex = pxBuffer->uiSize - pxBuffer->uiCount - 1;

			vPort_SPI_WRT_DR_NO_WAIT(ucUnitNumber, pxBuffer->pcArr[uiIndex]);
			pxBuffer->uiCount++;
			vPORT_SPI_ENABLE_TXE_INTERRUPT(ucUnitNumber);
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
		pxSPIBufferArr[i].uiCount = 0;
		pxSPIBufferArr[i].uiSize = 0;

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

		/*
		 * Enable TC interrupt in the interrupt controller, and set its priority
		 * to the maximum possible (i.e.: configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY).
		 * TODO: is this the best priority to choose?
		 */
		vPort_Interrupt_setPriority(pxPortInterruptSpiIrqNumberArr[i], configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
		vPort_Interrupt_enableIRQ(pxPortInterruptSpiIrqNumberArr[i]);
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

static uint8_t fads = 0;
void vHOS_SPI_send(uint8_t ucUnitNumber, int8_t* pcArr, uint32_t uiSize)
{
	pxSPIBufferArr[ucUnitNumber].pcArr = pcArr;
	pxSPIBufferArr[ucUnitNumber].uiCount = 0;
	pxSPIBufferArr[ucUnitNumber].uiSize = uiSize;

	fads++;
	vTaskResume(pxSPITaskHandleArr[ucUnitNumber]);
}

inline __attribute__((always_inline))
SemaphoreHandle_t xHOS_SPI_getTransferMutexHandle(uint8_t ucUnitNumber)
{
	return pxSPITransferMutexArr[ucUnitNumber];
}

inline __attribute__((always_inline))
SemaphoreHandle_t xHOS_SPI_getUnitMutexHandle(uint8_t ucUnitNumber)
{
	return pxSPIUnitMutexArr[ucUnitNumber];
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
	vPORT_SPI_DISABLE_TXE_INTERRUPT(n);									\
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

/*******************************************************************************
 * HW configuration:
 ******************************************************************************/
/*	TODO: this part is not completed yet, it is barley working!	*/
#define xHW_CONF(n)                                    \
static const xHOS_SPI_HW_Conf_t xHWConf##n = {         \
	configHOS_SPI_##n##_ENABLE_FULL_DUPLEX		,      \
	configHOS_SPI_##n##_SET_FRAME_FORMAT_8_BIT	,      \
	configHOS_SPI_##n##_SET_FRAME_FORMAT_16_BIT	,      \
	configHOS_SPI_##n##_SET_DIRECTION_LSBIT_FIRST	,      \
	configHOS_SPI_##n##_SET_DIRECTION_LSBYTE_FIRST	,      \
	configHOS_SPI_##n##_IS_MASTER				,      \
	configHOS_SPI_##n##_ENABLE_MOSI				,      \
	configHOS_SPI_##n##_ENABLE_MISO				,      \
	configHOS_SPI_##n##_ENABLE_NSS				,      \
	configHOS_SPI_##n##_AFIO_MAP_NUMBER			,      \
	configHOS_SPI_##n##_COM_MODE				,      \
	configHOS_SPI_##n##_BAUDRATE_PRESCALER             \
};

void vHOS_SPI_initHardware(uint8_t ucUnitNumber, xHOS_SPI_HW_Conf_t* pxHWConf)
{
	if (pxHWConf->ucFullDuplexEn)
		vPort_SPI_setFullDuplex(ucUnitNumber);

	if (pxHWConf->ucFrameFormat8)
		vPort_SPI_setFrameFormat8Bit(ucUnitNumber);
	else
		{/*vPort_SPI_setFrameFormat16Bit(ucUnitNumber);*/}

	if (pxHWConf->ucLSBitFirst)
		vPort_SPI_setLSBFirst(ucUnitNumber);
	else
		vPort_SPI_setMSBFirst(ucUnitNumber);

	if (pxHWConf->ucLSByteFirst)
		vHOS_SPI_setByteDirection(ucUnitNumber, ucHOS_SPI_BYTE_DIRECTION_LSBYTE_FIRST);
	else
		vHOS_SPI_setByteDirection(ucUnitNumber, ucHOS_SPI_BYTE_DIRECTION_MSBYTE_FIRST);

	vPort_SPI_setBaudratePrescaler(ucUnitNumber, pxHWConf->usBaudratePrescaler);

	if (pxHWConf->ucIsMaster)
		vPort_SPI_enableMasterMode(ucUnitNumber);

	vPort_SPI_setComMode(ucUnitNumber, pxHWConf->ucComMode);

	vPort_SPI_enable(ucUnitNumber);

	vPort_AFIO_mapSpi(ucUnitNumber, pxHWConf->ucAFIOMapNumber);

	vPort_GPIO_initSpiPins(	ucUnitNumber,
							pxHWConf->ucAFIOMapNumber,
							pxHWConf->ucNssEn,
							pxHWConf->ucMISOEn,
							pxHWConf->ucMOSIEn	);
}

void vHOS_SPI_initAllUnitsHardware(void)
{
#if (configHOS_SPI_NUMBER_OF_UNITS > 0)
	xHW_CONF(0);
	vHOS_SPI_initHardware(0, (xHOS_SPI_HW_Conf_t*)&xHWConf0);
#endif
}



#endif	/*	configHOS_SPI_EN	*/
