/*
 * RunTimeStackAlalyzer.h
 *
 *  Created on: Sep 25, 2023
 *      Author: Ali Emad
 */

#ifndef COTS_OS_INC_LIB_RUNTIMESTACKALALYZER_RUNTIMESTACKALALYZER_H_
#define COTS_OS_INC_LIB_RUNTIMESTACKALALYZER_RUNTIMESTACKALALYZER_H_

#include "LIB/RunTimeStackAlalyzer/RunTimeStackAlalyzer_Config.h"

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
void vLIB_RunTimeStackAlalyzer_addTask(	char* pcTaskName,
										StackType_t* pxTaskStack,
										uint32_t uiStackSize	);

/*
 * Gets stack usage (in words).
 *
 * Notes:
 * 		-	if "pcTaskName" was not found in the added tasks, function will return -1.
 */
int32_t iLIB_RunTimeStackAlalyzer_getTaskStackUsageInWords(char* pcTaskName);



#endif /* COTS_OS_INC_LIB_RUNTIMESTACKALALYZER_RUNTIMESTACKALALYZER_H_ */
