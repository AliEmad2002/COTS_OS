/*
 * PWMMeasure_Config.h
 *
 *  Created on: Sep 20, 2023
 *      Author: Ali Emad
 */

#ifndef COTS_OS_INC_HAL_PWMMEASURE_PWMMEASURE_CONFIG_H_
#define COTS_OS_INC_HAL_PWMMEASURE_PWMMEASURE_CONFIG_H_


#define uiCONF_PWM_MEASURE_EXTI_PRI		(configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1)

#define uiCONF_PWM_MEASURE_RTOS_PRI		(configHOS_HARD_REAL_TIME_TASK_PRI)

/*
 * Number of idle periods the signal should make first before marking it of a zero frequency.
 */
#define uiCONF_PWM_MEASURE_NUMBR_OF_IDLE_PEERIODS_TILL_ZERO_HZ		(10)


#endif /* COTS_OS_INC_HAL_PWMMEASURE_PWMMEASURE_CONFIG_H_ */
