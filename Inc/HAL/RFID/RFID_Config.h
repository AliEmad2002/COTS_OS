/*
 * RFID_Config.h
 *
 *  Created on: Sep 15, 2023
 *      Author: Ali Emad
 */

#ifndef COTS_OS_INC_HAL_RFID_RFID_CONFIG_H_
#define COTS_OS_INC_HAL_RFID_RFID_CONFIG_H_

/*
 * Number of reads which could be temporarily buffered in RFID handle's queue.
 * If driver received one more reading hen the queue is full, it automatically
 * deletes front of the queue (first-in element) and adds the new ID.
 */
#define uiCONF_RFID_READ_QUEUE_LEN		10

/*
 * Un-comment only one of the following lines based on the module being used.
 */
//#define uiCONF_RFID_TYPE_RDM6300
#define uiCONF_RFID_TYPE_UNKNOWN_125KHZ

#endif /* COTS_OS_INC_HAL_RFID_RFID_CONFIG_H_ */
