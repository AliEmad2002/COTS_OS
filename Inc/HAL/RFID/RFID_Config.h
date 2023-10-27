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
#define uiCONF_RFID_READ_QUEUE_LEN			10

/*
 * Timeout between two consecutive reads on the RDM6300 module.
 */
#define uiCONF_RFID_RDM6300_TIMEOUT_MS		200

#endif /* COTS_OS_INC_HAL_RFID_RFID_CONFIG_H_ */
