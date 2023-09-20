/*
 * PWMDutyMeasure.h
 *
 *  Created on: Sep 20, 2023
 *      Author: Ali Emad
 *
 * This driver measures frequency and duty cycle of a PWM signal using both EXTI
 * and HW-based timestamping. It is of course much slower and has more error than
 * the HW-based input / capture unit. Although, it becomes useful when masuring
 * multiple signals - more than what HW timers could handle -.
 *
 * Maximum measurement frequency depends on multiple factors such as core clock,
 * driver's RTOS priority, EXTI priority. These priorities are used configured
 * in the config.h file.
 *
 * Test on STM32F103C8T6: maximum measurable frequency is about 15kHZ. (The number
 * may be less, based on CPU load of the application program).
 */

#ifndef COTS_OS_INC_HAL_PWMMEASURE_PWMMEASURE_H_
#define COTS_OS_INC_HAL_PWMMEASURE_PWMMEASURE_H_

#include "HAL/PWMMeasure/PWMMeasure_Config.h"

typedef struct{
	/*	PUBLIC	*/
	uint8_t ucPort;
	uint8_t ucPin;

	/*	PRIVATE	*/
	TaskHandle_t xTask;
	StackType_t pxTaskStack[configMINIMAL_STACK_SIZE];
	StaticTask_t xTaskStatic;

	SemaphoreHandle_t xNewMeasurementSemaphore;
	StaticSemaphore_t xNewMeasurementSemaphoreStatic;

	uint64_t ulPrevRisingTime;
	uint32_t uiPeriodTime;
	uint32_t uiActiveTime;
	uint32_t uiNumberOfIdlePeriods;
	uint8_t ucPrevEdge;
}xHOS_PWMDutyMeasure_t;


/*
 * Initializes handle.
 *
 * Notes:
 * 		-	"HWTimestamp" driver must be initialized first.
 *
 * 		-	Configurations in "PWMMeasure_Config.h" must be set to valid values.
 *
 * 		-	"ucPort" & "ucPin" of the passed handle must be set to valid values.
 *
 * 		-	EXTI interrupts must be enabled in "Port_Interrupt.h".
 * 			(i.e.: define the "ucPORT_INTERRUPT_IRQ_DEF_EXTI" macro)
 */
void vHOS_PWMMeasure_init(xHOS_PWMDutyMeasure_t* pxHandle);

/*
 * Returns active duty cycle of the signal.
 *
 * Notes:
 * 		-	Return value expresses the mapping of duty cycle between 0 and 2 ^ 32.
 *
 * 		-	If the input signal is too fast to be handled, this function returns
 * 			maximum duty.
 */
uint32_t uiHOS_PWMMeasure_getDuty(xHOS_PWMDutyMeasure_t* pxHandle);

/*
 * Returns signal's frequency.
 *
 * Notes:
 * 		-	If the input signal is too fast to be handled, this function returns
 * 			maximum frequency.
 */
uint32_t uiHOS_PWMMeasure_getFreq(xHOS_PWMDutyMeasure_t* pxHandle);





#endif /* COTS_OS_INC_HAL_PWMMEASURE_PWMMEASURE_H_ */
