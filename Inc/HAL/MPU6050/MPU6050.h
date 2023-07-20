/*
 * MPU6050.h
 *
 *  Created on: Jul 19, 2023
 *      Author: Ali Emad
 */

#ifndef COTS_OS_INC_HAL_MPU6050_MPU6050_H_
#define COTS_OS_INC_HAL_MPU6050_MPU6050_H_

typedef struct{
	uint8_t ucI2CUnitNumber;

	uint8_t ucAdd0State : 1;

	uint8_t ucIntPort : 4;
	uint8_t ucIntPin  : 4;
}xHOS_MPU6050_t;

typedef struct{
	uint8_t ucXAccelPassed : 1;
	uint8_t ucYAccelPassed : 1;
	uint8_t ucZAccelPassed : 1;
	uint8_t ucXGyroPassed : 1;
	uint8_t ucYGyroPassed : 1;
	uint8_t ucZGyroPassed : 1;
}xHOS_MPU6050_testResult_t;

/*
 * This function must be called after scheduler start.
 */
void vHOS_MPU6050_init(xHOS_MPU6050_t* pxHandle);

/*
 * Runs self test for all 6 measurement units.
 */
uint8_t ucHOS_MPU6050_selfTest(	xHOS_MPU6050_t* pxHandle,
								xHOS_MPU6050_testResult_t* pxResults	);

/*
 *
 */



#endif /* COTS_OS_INC_HAL_MPU6050_MPU6050_H_ */
