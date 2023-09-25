/*
 * RTOS_PRI_Config.h
 *
 *  Created on: Jun 28, 2023
 *      Author: Ali Emad
 */

#ifndef HAL_OS_INC_RTOS_PRI_CONFIG_H_
#define HAL_OS_INC_RTOS_PRI_CONFIG_H_

#include "FreeRTOS.h"

/*
 * Priority of initialization task/s.
 * Must be higher than any other priority in the system, such that no tasks run
 * be fore initialization is done.
 */
#define configHOS_INIT_REAL_TIME_TASK_PRI		(configMAX_PRIORITIES - 1)

/*
 * Priority of HW-Time handling task.
 * Must be higher that other tasks in order to achieve accurate HW-timing.
 */
#define configHOS_HWTIME_TASK_PRI				(configMAX_PRIORITIES - 2)

#define configHOS_HARD_REAL_TIME_TASK_PRI		(configMAX_PRIORITIES - 3)

#define configHOS_MID_REAL_TIME_TASK_PRI		(configMAX_PRIORITIES - 4)

#define configHOS_SOFT_REAL_TIME_TASK_PRI		(configMAX_PRIORITIES - 5)

#define configHOS_IDLE_REAL_TIME_TASK_PRI		(0)	// i.e.:	(configMAX_PRIORITIES - 6)



#endif /* HAL_OS_INC_RTOS_PRI_CONFIG_H_ */
