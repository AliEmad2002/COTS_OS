/*
 * RDM6300_Config.h
 *
 *  Created on: Sep 15, 2023
 *      Author: Ali Emad
 */

#ifndef COTS_OS_INC_HAL_RDM6300_RDM6300_CONFIG_H_
#define COTS_OS_INC_HAL_RDM6300_RDM6300_CONFIG_H_

/*
 * Number of reads which could be temporarily buffered in RDM6300 handle's queue.
 * If driver received one more reading hen the queue is full, it automatically
 * deletes front of the queue (first-in element) and adds the new ID.
 */
#define uiCONF_RDM6300_READ_QUEUE_LEN		10


#endif /* COTS_OS_INC_HAL_RDM6300_RDM6300_CONFIG_H_ */
