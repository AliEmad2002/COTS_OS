/*
 * RunTimeStackAlalyzer.h
 *
 *  Created on: Sep 25, 2023
 *      Author: Ali Emad
 *
 * User may add "pxStackAnalysisArr" to the "expressions" or "live expressions"
 * debugging tool to monitor tasks usage at runtime.
 *
 * Note:
 * 		-	Same functionality provided by this driver is already available in the
 * 			"FreeRTOS task list" tool of STM32CubeIDE. Use this driver only if
 * 			not using STM32CubeIDE.
 */

#ifndef COTS_OS_INC_LIB_RUNTIMESTACKALALYZER_RUNTIMESTACKALALYZER_H_
#define COTS_OS_INC_LIB_RUNTIMESTACKALALYZER_RUNTIMESTACKALALYZER_H_

#include "FreeRTOS.h"
#include "task.h"

#include "LIB/RunTimeStackAlalyzer/RunTimeStackAlalyzer_Config.h"

#if ucCONF_RUN_TIME_STACK_ALALYZER_ENABLE

/*
 * Initializes the driver.
 */
void vLIB_RunTimeStackAlalyzer_init(void);

/*
 * Adds a task to the analyzer.
 *
 * Notes:
 * 		-	Analyzer would only analyze tasks added to it.
 */
void vLIB_RunTimeStackAlalyzer_addTask(TaskHandle_t xTask);

#endif

#endif /* COTS_OS_INC_LIB_RUNTIMESTACKALALYZER_RUNTIMESTACKALALYZER_H_ */
