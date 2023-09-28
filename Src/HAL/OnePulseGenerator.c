/*
 * OnePulseGenerator.c
 *
 *  Created on: Sep 26, 2023
 *      Author: Ali Emad
 */

/*	LIB	*/
#include "stdint.h"

/*	RTOS	*/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/*	MCAL	*/
#include "MCAL_Port/Port_Interrupt.h"
#include "MCAL_Port/Port_GPIO.h"

/*	SELF	*/
#include "HAL/OnePulseGenerator/OnePulseGenerator.h"

/*******************************************************************************
 * Callback:
 ******************************************************************************/
static void vOvfCallback(void* pvParams)
{
	xHOS_OnePulseGenerator_t* pxHandle = (xHOS_OnePulseGenerator_t*)pvParams;

	/*	Disable last enabled channel	*/
	vPORT_TIM_DISABLE_CC_CHANNEL(pxHandle->ucTimerUnitNumber, pxHandle->ucLastEnabledChannel);

	/*	Release unit's semaphore	*/
	BaseType_t xHigherPriTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(pxHandle->xSemaphore, &xHigherPriTaskWoken);
	portYIELD_FROM_ISR(xHigherPriTaskWoken);
}

/*******************************************************************************
 * API functions:
 ******************************************************************************/
/*
 * See header for info.
 */
void vHOS_OnePulseGenerator_init(	xHOS_OnePulseGenerator_t* pxHandle,
									uint32_t uiFreq	)
{
	/*	Create unit's semaphore	*/
	pxHandle->xSemaphore = xSemaphoreCreateBinaryStatic(&pxHandle->xSemaphoreStatic);
	xSemaphoreGive(pxHandle->xSemaphore);

	/*	No channels were enabled before	*/
	pxHandle->ucLastEnabledChannel = 0;

	/*	Initialize timer update interrupt	*/
	vPort_TIM_setOvfCallback(	pxHandle->ucTimerUnitNumber,
								vOvfCallback,
								(void*)pxHandle	);

	vPORT_TIM_ENABLE_OVF_INTERRUPT(pxHandle->ucTimerUnitNumber);

	VPORT_INTERRUPT_SET_PRIORITY(
		pxPortInterruptTimerOvfIrqNumberArr[pxHandle->ucTimerUnitNumber],
		configLIBRARY_LOWEST_INTERRUPT_PRIORITY	);

	vPORT_INTERRUPT_ENABLE_IRQ(
		pxPortInterruptTimerOvfIrqNumberArr[pxHandle->ucTimerUnitNumber]	);

	/*	Initialize timer HW	*/
	vPort_TIM_initOPM(pxHandle->ucTimerUnitNumber, uiFreq);
}

/*
 * See header for info.
 */
void vHOS_OnePulseGenerator_initChannel(	xHOS_OnePulseGenerator_t* pxHandle,
											uint8_t ucChannelNumber	)
{
	/*	Initialize channel's GPIO mapping	*/
	vPort_GPIO_initTimerChannelPinAsOutput(	pxHandle->ucTimerUnitNumber,
											ucChannelNumber,
											0	);
}

/*
 * See header for info.
 */
void vHOS_OnePulseGenerator_generate(	xHOS_OnePulseGenerator_t* pxHandle,
												uint8_t ucChannelNumber,
												uint32_t uiHighTimeNanoSec	)
{
	/*	Wait for previous pulse on the same unit to end	*/
	xSemaphoreTake(pxHandle->xSemaphore, portMAX_DELAY);

	/*	Enable channel	*/
	vPORT_TIM_ENABLE_CC_CHANNEL(pxHandle->ucTimerUnitNumber, ucChannelNumber);
	pxHandle->ucLastEnabledChannel = ucChannelNumber;

	/*	Generate pulse	*/
	vPort_TIM_generateOnePulse(	pxHandle->ucTimerUnitNumber,
								ucChannelNumber,
								uiHighTimeNanoSec	);
}














