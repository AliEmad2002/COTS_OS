/*
 * RTOS_PRI_Config.h
 *
 *  Created on: Jun 28, 2023
 *      Author: Ali Emad
 */

#ifndef HAL_OS_INC_RTOS_PRI_CONFIG_H_
#define HAL_OS_INC_RTOS_PRI_CONFIG_H_

#include "FreeRTOS.h"

#define configHOS_HARD_REAL_TIME_TASK_PRI		(configMAX_PRIORITIES - 1)

#define configHOS_SOFT_REAL_TIME_TASK_PRI		(configMAX_PRIORITIES - 2)



#endif /* HAL_OS_INC_RTOS_PRI_CONFIG_H_ */
