/*
 * I2C.h
 *
 *  Created on: Jul 11, 2023
 *      Author: Ali Emad
 */

#ifndef COTS_OS_INC_HAL_I2C_I2C_H_
#define COTS_OS_INC_HAL_I2C_I2C_H_

typedef struct{
	uint8_t ucUnitNumber;
	uint16_t usAddress;
	uint8_t ucIs7BitAddress;
	uint8_t* pucTxArr;
	uint32_t uiTxSize;
	uint8_t* pucRxArr;
	uint32_t uiRxSize;
}xHOS_I2C_transreceiveParams_t;

void vHOS_I2C_enable(uint8_t ucUnitNumber);

void vHOS_I2C_disable(uint8_t ucUnitNumber);

/*
 * To avoid resulting in bus error (case multi-masters are on the bus), this function
 * must be a critical section.
 * ("taskENTER_CRITICAL()" and "taskEXIT_CRITICAL()" are called inside)
 */
uint8_t ucHOS_I2C_masterTransReceive(xHOS_I2C_transreceiveParams_t* pxParams);

/*
 * To avoid missing a byte, this function must be a critical section.
 * ("taskENTER_CRITICAL()" and "taskEXIT_CRITICAL()" are called inside)
 */
uint8_t ucHOS_I2C_slaveReceive(	uint8_t ucUnitNumber,
								uint8_t* pucArr,
								uint32_t uiMaxSize,
								uint32_t* puiSize	);



#endif /* COTS_OS_INC_HAL_I2C_I2C_H_ */
