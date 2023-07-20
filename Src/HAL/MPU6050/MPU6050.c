/*
 * MPU6050.c
 *
 *  Created on: Jul 19, 2023
 *      Author: Ali Emad
 */

/*	LIB	*/
#include "stdint.h"
#include "stdlib.h"

/*	MCAL (ported)	*/

/*	OS	*/
#include "FreeRTOS.h"
#include "task.h"

/*	HAL	*/
#include "HAL/I2C/I2C.h"

/*	SELF	*/
#include "HAL/MPU6050/MPU6050_private.h"
#include "HAL/MPU6050/MPU6050.h"



/*******************************************************************************
 * Helping functions / macros:
 ******************************************************************************/
static uint8_t ucEditReg(	xHOS_MPU6050_t* pxHandle,
							uint8_t ucAddress,
							uint8_t ucMask,
							uint8_t ucVal	)
{
	uint8_t ucReg;
	uint8_t ucSuccessful;
	uint8_t pucAddValArr[2];

	/*	Prepare transreceive params	*/
	xHOS_I2C_transreceiveParams_t xTRParams = {
		.ucUnitNumber = pxHandle->ucI2CUnitNumber,
		.usAddress = MPU6050_ADDRESS0 | pxHandle->ucAdd0State,
		.ucIs7BitAddress = 1,
		.pucTxArr = &ucAddress,
		.uiTxSize = 1,
		.pucRxArr = &ucReg,
		.uiRxSize = 1
	};

	/*	Read register	*/
	ucSuccessful = ucHOS_I2C_masterTransReceive(&xTRParams);
	if (!ucSuccessful)
		return 0;

	/*	Clear the masked bits	*/
	ucReg = ucReg & (~ucMask);

	/*	Assure "ucVal"'s non masked bits are zeros	*/
	ucVal = ucVal & ucMask;

	/*	Write input value on register value	*/
	ucReg = ucReg | ucVal;

	/*	Prepare transreceive params for the second time	*/
	pucAddValArr[0] = ucAddress;
	pucAddValArr[1] = ucReg;

	xTRParams.pucTxArr = pucAddValArr;
	xTRParams.uiTxSize = 2;
	xTRParams.uiRxSize = 0;

	/*	Write data to the MPU6050	*/
	ucSuccessful = ucHOS_I2C_masterTransReceive(&xTRParams);
	if (!ucSuccessful)
		return 0;

	return 1;
}

static uint8_t ucReadRawGyroData(	xHOS_MPU6050_t* pxHandle,
									xHOS_MPU6050_rawMeasurements_t* pxData	)
{
	uint8_t ucSuccessful;

	/*	Prepare transreceive params	*/
	uint8_t ucAddress = MPU6050_REG_GYRO_XOUT_H;

	xHOS_I2C_transreceiveParams_t xTRParams = {
		.ucUnitNumber = pxHandle->ucI2CUnitNumber,
		.usAddress = MPU6050_ADDRESS0 | pxHandle->ucAdd0State,
		.ucIs7BitAddress = 1,
		.pucTxArr = &ucAddress,
		.uiTxSize = 1,
		.pucRxArr = (uint8_t*)pxData,
		.uiRxSize = 6
	};

	/*	Brust read registers	*/
	ucSuccessful = ucHOS_I2C_masterTransReceive(&xTRParams);
	if (!ucSuccessful)
		return 0;

	return 1;
}

static uint8_t ucReadRawAccelData(	xHOS_MPU6050_t* pxHandle,
									xHOS_MPU6050_rawMeasurements_t* pxData	)
{
	uint8_t ucSuccessful;

	/*	Prepare transreceive params	*/
	uint8_t ucAddress = MPU6050_REG_ACCEL_XOUT_H;

	xHOS_I2C_transreceiveParams_t xTRParams = {
		.ucUnitNumber = pxHandle->ucI2CUnitNumber,
		.usAddress = MPU6050_ADDRESS0 | pxHandle->ucAdd0State,
		.ucIs7BitAddress = 1,
		.pucTxArr = &ucAddress,
		.uiTxSize = 1,
		.pucRxArr = (uint8_t*)pxData,
		.uiRxSize = 6
	};

	/*	Brust read registers	*/
	ucSuccessful = ucHOS_I2C_masterTransReceive(&xTRParams);
	if (!ucSuccessful)
		return 0;

	return 1;
}

/*******************************************************************************
 * API functions:
 ******************************************************************************/
/*	See header for info	*/
void vHOS_MPU6050_init(xHOS_MPU6050_t* pxHandle)
{
	/*	Voltage stabilization delay	*/
	vTaskDelay(pdMS_TO_TICKS(200));
}

/*	See header for info	*/
uint8_t ucHOS_MPU6050_selfTest(	xHOS_MPU6050_t* pxHandle,
								xHOS_MPU6050_testResult_t* pxResults	)
{
	uint8_t ucSuccessful;
	xHOS_MPU6050_rawMeasurements_t xRawGyroDataTestOn;
	xHOS_MPU6050_rawMeasurements_t xRawAccelDataTestOn;
	xHOS_MPU6050_rawMeasurements_t xRawGyroDataTestOff;
	xHOS_MPU6050_rawMeasurements_t xRawAccelDataTestOff;

	/*	Activate self test for all 6-units	*/
	ucSuccessful = ucEditReg(pxHandle, MPU6050_REG_GYRO_CONFIG, 0b11100000, 0b11100000);
	if (!ucSuccessful)
		return 0;

	ucSuccessful = ucEditReg(pxHandle, MPU6050_REG_ACCEL_CONFIG, 0b11100000, 0b11100000);
	if (!ucSuccessful)
		return 0;

	/*	Read outputs of all 6-units	*/
	ucSuccessful = ucReadRawGyroData(pxHandle, &xRawGyroDataTestOn);
	if (!ucSuccessful)
		return 0;

	ucSuccessful = ucReadRawAccelData(pxHandle, &xRawAccelDataTestOn);
	if (!ucSuccessful)
		return 0;

	/*	De-activate self test for all 6-units	*/
	ucSuccessful = ucEditReg(pxHandle, MPU6050_REG_GYRO_CONFIG, 0b11100000, 0);
	if (!ucSuccessful)
		return 0;

	ucSuccessful = ucEditReg(pxHandle, MPU6050_REG_ACCEL_CONFIG, 0b11100000, 0);
	if (!ucSuccessful)
		return 0;

	/*	Read outputs of all 6-units	*/
	ucSuccessful = ucReadRawGyroData(pxHandle, &xRawGyroDataTestOff);
	if (!ucSuccessful)
		return 0;

	ucSuccessful = ucReadRawAccelData(pxHandle, &xRawAccelDataTestOff);
	if (!ucSuccessful)
		return 0;

	/*	Calculate self test response and decide if self test is passed or not for all 6-units */
	pxResults->ucXGyroPassed = abs(xRawGyroDataTestOn.sX - xRawGyroDataTestOff.sX) < 14;
	pxResults->ucYGyroPassed = abs(xRawGyroDataTestOn.sY - xRawGyroDataTestOff.sY) < 14;
	pxResults->ucZGyroPassed = abs(xRawGyroDataTestOn.sZ - xRawGyroDataTestOff.sZ) < 14;
	pxResults->ucXAccelPassed = abs(xRawAccelDataTestOn.sX - xRawAccelDataTestOff.sX) < 14;
	pxResults->ucYAccelPassed = abs(xRawAccelDataTestOn.sY - xRawAccelDataTestOff.sY) < 14;
	pxResults->ucZAccelPassed = abs(xRawAccelDataTestOn.sZ - xRawAccelDataTestOff.sZ) < 14;

	/*	No communication error occurred	*/
	return 1;
}

















