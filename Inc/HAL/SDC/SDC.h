/*
 * SDC_interface.h
 *
 *  Created on: Apr 24, 2023
 *      Author: Ali Emad
 *
 * Notes: (Following are general notes, more notes can be found prior to each function)
 * 		-	This driver can handle multiple SDCs.
 *
 * 		-	This driver assumes every SDC has one partition. Other wise it would
 * 			operate on the 1st partition in the MBR.
 *
 */

#ifndef INCLUDE_HAL_SDC_SDC_INTERFACE_H_
#define INCLUDE_HAL_SDC_SDC_INTERFACE_H_

#include "HAL/SDC/SDC_Helper.h"


/*******************************************************************************
 * SD-Card
 ******************************************************************************/
typedef struct{
	/*		PRIVATE		*/
	/*	SDC's main buffer (other buffers are created for streams).	*/
	xHOS_SDC_Block_Buffer_t xBuffer;

	/*	Info of SDC's file allocation table.	*/
	xHOS_SDC_FAT_t xFat;

	/*	LBA of first root directory cluster in SD-card's partition.	*/
	uint32_t uiClustersBeginLba;

	/*	Number of sectors per cluster. (configured in the volume ID)	*/
	uint8_t ucSectorsPerCluster;

	/*
	 * Binary semaphore for handle's initialization, to synchronize tasks which
	 * want to operate on this handle, such that they don't start operating on
	 * it until initialization is done. Could be accessed using the following
	 * function: "ucHOS_SDC_waitForInitCompletion()".
	 */
	SemaphoreHandle_t xInitCompletionSemaphore;
	StaticSemaphore_t xInitCompletionSemaphoreStatic;

	/*		PUBLIC		*/
	/*
	 * The following 4 variables must be set once before initialization, and must
	 * not be changed after that.
	 */
	uint8_t ucSpiUnitNumber;
	uint8_t ucCsPin;
	uint8_t ucCsPort;
	uint8_t ucIsCrcEnabled;

	/*	Version of the SDC connected to the handle. (read only)	*/
	xHOS_SDC_Version_t xVer;

	uint32_t uiNumberOfSectors;

	uint32_t uiPartitionBeginLba;

	/*
	 * Mutex of the SDC handle. This must be taken before doing any operations
	 * on the handle.
	 */
	SemaphoreHandle_t xMutex;
	StaticSemaphore_t xMutexStatic;
}xHOS_SDC_t;



/*******************************************************************************
 * API functions:
 ******************************************************************************/
/**
 * Following, all functions named starting by "keepTrying" would do the same functionality
 * of their previous ones, except that these function would do the following loop continuously
 * until timeout is reached:
 * 		-	Try the function with a maximum of three chances of failure.
 * 		-	If the function still fails three times, reset the SD-card.
 * 		-	If the required functionality is done successfully, it returns.
 **/

/*
 * Initializes SDC handle.
 *
 * Notes:
 * 		-	All configuration parameters in the passed "xHOS_SDC_t" handle must
 * 			be initialized with valid values.
 */
void vHOS_SDC_init(xHOS_SDC_t* pxSdc);

/*
 * Runs the initialize flow, and initializes the partition. To be ready for
 * "SD_Stream" use. (Assumes the card has only one partition. if more, it takes
 * the first).
 *
 * Notes:
 * 		-	"ucHOS_SDC_init()" must be called first.
 *
 * 		-	This function must be called after scheduler start.
 *
 * 		-	This function must be called before using any of the "SD_Stream"
 * 			functions. This could be achieved using "ucHOS_SDC_waitForInitCompletion()"
 * 			in the other tasks to force them block until partition is initialized.
 */
uint8_t ucHOS_SDC_initPartition(xHOS_SDC_t* pxSdc, TickType_t xTimeout);

uint8_t ucHOS_SDC_keepTryingInitPartition(xHOS_SDC_t* pxSdc, TickType_t xTimeout);

/*
 * This function is used to synchronize other tasks with the task that initializes
 * SDC's partition for the first time.
 */
uint8_t ucHOS_SDC_waitForInitCompletion(xHOS_SDC_t* pxSdc, TickType_t xTimeout);






#endif /* INCLUDE_HAL_SDC_SDC_INTERFACE_H_ */
