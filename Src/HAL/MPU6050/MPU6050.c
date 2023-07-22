/*
 * MPU6050.c
 *
 *  Created on: Jul 19, 2023
 *      Author: Ali Emad
 */

/*	LIB	*/
#include "stdint.h"
#include "stdlib.h"
#include "stdio.h"
#include "math.h"

/*	MCAL (ported)	*/
#include "MCAL_Port/Port_Breakpoint.h"
#include "MCAL_Port/Port_DIO.h"

/*	OS	*/
#include "FreeRTOS.h"
#include "task.h"
#include "RTOS_PRI_Config.h"

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

static uint8_t ucReadGyroSelfTestRegisters(	xHOS_MPU6050_t* pxHandle,
											xHOS_MPU6050_selfTestRegs_t* pxRegs	)
{
	uint8_t ucSuccessful;

	/*	Prepare transreceive params	*/
	uint8_t ucAddress = MPU6050_REG_SELF_TEST_X;

	xHOS_I2C_transreceiveParams_t xTRParams = {
		.ucUnitNumber = pxHandle->ucI2CUnitNumber,
		.usAddress = MPU6050_ADDRESS0 | pxHandle->ucAdd0State,
		.ucIs7BitAddress = 1,
		.pucTxArr = &ucAddress,
		.uiTxSize = 1,
		.pucRxArr = (uint8_t*)pxRegs,
		.uiRxSize = 3
	};

	/*	Brust read registers	*/
	ucSuccessful = ucHOS_I2C_masterTransReceive(&xTRParams);
	if (!ucSuccessful)
		return 0;

	return 1;
}

static uint8_t ucReadAccelSelfTestRegisters(	xHOS_MPU6050_t* pxHandle,
												xHOS_MPU6050_selfTestRegs_t* pxRegs	)
{
	uint8_t ucSuccessful;

	/*	Prepare transreceive params	*/
	uint8_t ucAddress = MPU6050_REG_SELF_TEST_X;
	uint8_t pucRegArr[4];

	xHOS_I2C_transreceiveParams_t xTRParams = {
		.ucUnitNumber = pxHandle->ucI2CUnitNumber,
		.usAddress = MPU6050_ADDRESS0 | pxHandle->ucAdd0State,
		.ucIs7BitAddress = 1,
		.pucTxArr = &ucAddress,
		.uiTxSize = 1,
		.pucRxArr = pucRegArr,
		.uiRxSize = 4
	};

	/*	Brust read registers	*/
	ucSuccessful = ucHOS_I2C_masterTransReceive(&xTRParams);
	if (!ucSuccessful)
		return 0;

	/*	from the read registers, form-up the accel self test values	*/
	pxRegs->ucX = ((pucRegArr[3] >> 4) & 0b11) | ((pucRegArr[0] >> 3) & 0b11100);
	pxRegs->ucY = ((pucRegArr[3] >> 2) & 0b11) | ((pucRegArr[1] >> 3) & 0b11100);
	pxRegs->ucZ = ((pucRegArr[3] >> 0) & 0b11) | ((pucRegArr[2] >> 3) & 0b11100);

	return 1;
}

static uint8_t ucReadGyroRawMeasurement(	xHOS_MPU6050_t* pxHandle,
											xHOS_MPU6050_rawMeasurement_t* pxData	)
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

	/*	Swap bytes in each variable	*/
	pxData->sX = ((pxData->sX & 0xFF) << 8) | ((pxData->sX & 0xFF00) >> 8);
	pxData->sY = ((pxData->sY & 0xFF) << 8) | ((pxData->sY & 0xFF00) >> 8);
	pxData->sZ = ((pxData->sZ & 0xFF) << 8) | ((pxData->sZ & 0xFF00) >> 8);

	return 1;
}

static uint8_t ucReadAccelRawMeasurement(	xHOS_MPU6050_t* pxHandle,
											xHOS_MPU6050_rawMeasurement_t* pxData	)
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

	/*	Swap bytes in each variable	*/
	pxData->sX = ((pxData->sX & 0xFF) << 8) | ((pxData->sX & 0xFF00) >> 8);
	pxData->sY = ((pxData->sY & 0xFF) << 8) | ((pxData->sY & 0xFF00) >> 8);
	pxData->sZ = ((pxData->sZ & 0xFF) << 8) | ((pxData->sZ & 0xFF00) >> 8);

	return 1;
}

/*******************************************************************************
 * Task function:
 ******************************************************************************/
/*
 * This task is used to calculate tilt of an MPU6050 object (if tilt is enabled).
 */
static void vTask(void* pvParams)
{
	uint8_t ucSuccessful;
	xHOS_MPU6050_measurement_t xGyroMeasurement;
	xHOS_MPU6050_measurement_t xIntegration;

	xHOS_MPU6050_t* pxHandle = (xHOS_MPU6050_t*)pvParams;

	/*	Tilt calculation is initially disabled	*/
	vTaskSuspend(pxHandle->xTask);

	/*	Reset tilt values	*/
	pxHandle->xTilt.iX = 0;
	pxHandle->xTilt.iY = 0;
	pxHandle->xTilt.iZ = 0;

	/*	Rest previous gyro measurement	*/
	pxHandle->xPrevGyroMeasurement.iX = 0;
	pxHandle->xPrevGyroMeasurement.iY = 0;
	pxHandle->xPrevGyroMeasurement.iZ = 0;

	/*	Enable INT pin latch and INT flags clear on read	*/
	ucEditReg(pxHandle, MPU6050_REG_INT_PIN_CFG, 0b110000, 0b110000);

	/*	Enable INT on data ready	*/
	ucEditReg(pxHandle, MPU6050_REG_INT_ENABLE, 1, 1);


	TickType_t xLastWakeTime = xTaskGetTickCount();
	while(1)
	{
		/*	Wait for INT pin. (To synchronize task with samples taken by the MPU6050)	*/
		while(!ucPort_DIO_readPin(pxHandle->ucIntPort, pxHandle->ucIntPin));

		/*	Read gyro measurement	*/
		ucSuccessful = ucHOS_MPU6050_readGyroMeasurement(pxHandle, &xGyroMeasurement);
		configASSERT(ucSuccessful);

		/*	Integrate	*/
		xIntegration.iX = (pxHandle->xPrevGyroMeasurement.iX + xGyroMeasurement.iX) / 2;
		xIntegration.iY = (pxHandle->xPrevGyroMeasurement.iY + xGyroMeasurement.iY) / 2;
		xIntegration.iZ = (pxHandle->xPrevGyroMeasurement.iZ + xGyroMeasurement.iZ) / 2;

		/*	Ignore changes smaller than 0.01 degree, to avoid truncating error	*/
		if (xIntegration.iX > 100 || xIntegration.iX < -100)
			pxHandle->xTilt.iX += xIntegration.iX;

		if (xIntegration.iY > 100 || xIntegration.iY < -100)
			pxHandle->xTilt.iY += xIntegration.iY;

		if (xIntegration.iZ > 100 || xIntegration.iZ < -100)
			pxHandle->xTilt.iZ += xIntegration.iZ;

		/*	Check that angle is in the range: +-[0:180]	*/
		if(pxHandle->xTilt.iX > 180000000)		pxHandle->xTilt.iX -= 360000000;
		if(pxHandle->xTilt.iY > 180000000)		pxHandle->xTilt.iY -= 360000000;
		if(pxHandle->xTilt.iZ > 180000000)		pxHandle->xTilt.iZ -= 360000000;

		if(pxHandle->xTilt.iX < -180000000)		pxHandle->xTilt.iX += 360000000;
		if(pxHandle->xTilt.iY < -180000000)		pxHandle->xTilt.iY += 360000000;
		if(pxHandle->xTilt.iZ < -180000000)		pxHandle->xTilt.iZ += 360000000;

		/*	Update previous gyro measurement	*/
		pxHandle->xPrevGyroMeasurement.iX = xGyroMeasurement.iX;
		pxHandle->xPrevGyroMeasurement.iY = xGyroMeasurement.iY;
		pxHandle->xPrevGyroMeasurement.iZ = xGyroMeasurement.iZ;

		/*	Task is blocked until next sample time	*/
		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1));
	}
}

/*******************************************************************************
 * API functions:
 ******************************************************************************/
/*	See header for info	*/
uint8_t ucHOS_MPU6050_readReg(xHOS_MPU6050_t* pxHandle, uint8_t ucAddress, uint8_t* pucReg)
{
	uint8_t ucSuccessful;

	/*	Prepare transreceive params	*/
	xHOS_I2C_transreceiveParams_t xTRParams = {
		.ucUnitNumber = pxHandle->ucI2CUnitNumber,
		.usAddress = MPU6050_ADDRESS0 | pxHandle->ucAdd0State,
		.ucIs7BitAddress = 1,
		.pucTxArr = &ucAddress,
		.uiTxSize = 1,
		.pucRxArr = pucReg,
		.uiRxSize = 1
	};

	/*	Read register	*/
	ucSuccessful = ucHOS_I2C_masterTransReceive(&xTRParams);
	if (!ucSuccessful)
		return 0;

	return 1;
}

/*	See header for info	*/
uint8_t ucHOS_MPU6050_writeReg(xHOS_MPU6050_t* pxHandle, uint8_t ucAddress, uint8_t ucReg)
{
	uint8_t ucSuccessful;

	/*	Prepare transreceive params	*/
	uint8_t pucAddressValArr[2];
	pucAddressValArr[0] = ucAddress;
	pucAddressValArr[1] = ucReg;

	xHOS_I2C_transreceiveParams_t xTRParams = {
		.ucUnitNumber = pxHandle->ucI2CUnitNumber,
		.usAddress = MPU6050_ADDRESS0 | pxHandle->ucAdd0State,
		.ucIs7BitAddress = 1,
		.pucTxArr = pucAddressValArr,
		.uiTxSize = 2,
		.pucRxArr = NULL,
		.uiRxSize = 0
	};

	/*	Write register	*/
	ucSuccessful = ucHOS_I2C_masterTransReceive(&xTRParams);
	if (!ucSuccessful)
		return 0;

	return 1;
}

/*	See header for info	*/
void vHOS_MPU6050_init(xHOS_MPU6050_t* pxHandle)
{
	/*	Initialize INT pin if connected	*/
	if (pxHandle->ucIsIntConnected)
		vPort_DIO_initPinInput(pxHandle->ucIntPort, pxHandle->ucIntPin, 0);

	/*	Create task	*/
	static uint8_t ucCreatedObjectsCount = 0;
	char pcTaskName[configMAX_TASK_NAME_LEN];
	sprintf(pcTaskName, "MPU6050_%d", ucCreatedObjectsCount++);

	pxHandle->xTask = xTaskCreateStatic(	vTask,
											pcTaskName,
											configMINIMAL_STACK_SIZE,
											(void*)pxHandle,
											configHOS_HARD_REAL_TIME_TASK_PRI,
											pxHandle->puxTaskStack,
											&pxHandle->xTaskStatic	);
}

/*	See header for info	*/
void vHOS_MPU6050_enable(xHOS_MPU6050_t* pxHandle)
{
	/*	Exit sleep mode	*/
	ucEditReg(pxHandle, MPU6050_REG_PWR_MGMT_1, 1 << 6, 0);
}

/*	See header for info	*/
void vHOS_MPU6050_disable(xHOS_MPU6050_t* pxHandle)
{
	/*	Enter sleep mode	*/
	ucEditReg(pxHandle, MPU6050_REG_PWR_MGMT_1, 1 << 6, 1 << 6);
}

/*	See header for info	*/
uint8_t ucHOS_MPU6050_enableTiltCalculation(xHOS_MPU6050_t* pxHandle)
{
	if (!pxHandle->ucIsIntConnected)
		return 0;

	vTaskResume(pxHandle->xTask);

	return 1;
}

/*	See header for info	*/
void vHOS_MPU6050_disableTiltCalculation(xHOS_MPU6050_t* pxHandle)
{
	vTaskSuspend(pxHandle->xTask);
}

/*	See header for info	*/
void vHOS_MPU6050_enterDebuggerMode(xHOS_MPU6050_t* pxHandle)
{
	volatile uint8_t ucSuccessful;
	volatile uint8_t ucAddress;
	volatile uint8_t ucReg;
	volatile uint8_t ucRead = 0;
	volatile uint8_t ucWrite = 0;
	volatile uint8_t ucExit = 0;

	while(1)
	{
		//vPort_Breakpoint();
		if (ucRead)
		{
			/*	Read register	*/
			ucSuccessful = ucHOS_MPU6050_readReg(pxHandle, ucAddress, (uint8_t*)&ucReg);
			if (!ucSuccessful)	{	vPort_Breakpoint();	}
		}

		//vPort_Breakpoint();
		if (ucWrite)
		{
			/*	Write register	*/
			ucSuccessful = ucHOS_MPU6050_writeReg(pxHandle, ucAddress, ucReg);
			if (!ucSuccessful)	{	vPort_Breakpoint();	}
		}

		if (ucExit)
			break;
	}
}

/*	See header for info	*/
uint8_t ucHOS_MPU6050_gyroSelfTest(	xHOS_MPU6050_t* pxHandle,
									xHOS_MPU6050_testResult_t* pxResults	)
{
	uint8_t ucSuccessful;
	uint8_t ucReg;
	xHOS_MPU6050_rawMeasurement_t xRawData[2];
	xHOS_MPU6050_selfTestRegs_t xTestRegs;
	xHOS_MPU6050_factoryTrim_t xFT;

	/*	Check if gyro's full scale range is +-250dps	*/
	ucSuccessful = ucHOS_MPU6050_readReg(pxHandle, MPU6050_REG_GYRO_CONFIG, &ucReg);
	if (!ucSuccessful)
		return 0;

	if ((ucReg & 0b11000) != 0)
		return 0;

	/*	Activate self test for all 3 gyro units	*/
	ucSuccessful = ucEditReg(pxHandle, MPU6050_REG_GYRO_CONFIG, 0b11100000, 0b11100000);
	if (!ucSuccessful)
		return 0;

	/*	Read outputs of all 3 gyro units	*/
	ucSuccessful = ucReadGyroRawMeasurement(pxHandle, &xRawData[0]);
	if (!ucSuccessful)
		return 0;

	/*	De-activate self test for all 3 gyro units	*/
	ucSuccessful = ucEditReg(pxHandle, MPU6050_REG_GYRO_CONFIG, 0b11100000, 0);
	if (!ucSuccessful)
		return 0;

	/*	Read outputs of all 3 gyro units	*/
	ucSuccessful = ucReadGyroRawMeasurement(pxHandle, &xRawData[1]);
	if (!ucSuccessful)
		return 0;

	/*	Calculate self test response and store it in "xRawData[0]"	*/
	xRawData[0].sX = xRawData[0].sX - xRawData[1].sX;
	xRawData[0].sY = xRawData[0].sY - xRawData[1].sY;
	xRawData[0].sZ = xRawData[0].sZ - xRawData[1].sZ;

	/*	Read gyro self test registers	*/
	ucReadGyroSelfTestRegisters(pxHandle, &xTestRegs);

	/*	Calculate factory trim value	*/
	if (xTestRegs.ucX == 0)
		xFT.fX = 0;
	else
		xFT.fX = 25.0f * 131.0f * (float)pow(1.046, (double)(xTestRegs.ucX-1));

	if (xTestRegs.ucY == 0)
		xFT.fY = 0;
	else
		xFT.fY = 25.0f * 131.0f * (float)pow(1.046, (double)(xTestRegs.ucY-1));

	if (xTestRegs.ucZ == 0)
		xFT.fZ = 0;
	else
		xFT.fZ = 25.0f * 131.0f * (float)pow(1.046, (double)(xTestRegs.ucZ-1));

	/*
	 * calculate percentage change from factory trim, and decide whether gyro units
	 * pass the self test or not.
	 */
	pxResults->ucXPassed = fabs(((float)xRawData[0].sX - xFT.fX) / xFT.fX) < 14.0;
	pxResults->ucYPassed = fabs(((float)xRawData[0].sY - xFT.fY) / xFT.fY) < 14.0;
	pxResults->ucZPassed = fabs(((float)xRawData[0].sZ - xFT.fZ) / xFT.fZ) < 14.0;

	/*	No communication error occurred	*/
	return 1;
}

/*	See header for info	*/
uint8_t ucHOS_MPU6050_accelSelfTest(	xHOS_MPU6050_t* pxHandle,
									xHOS_MPU6050_testResult_t* pxResults	)
{
	uint8_t ucSuccessful;
	uint8_t ucReg;
	xHOS_MPU6050_rawMeasurement_t xRawData[2];
	xHOS_MPU6050_selfTestRegs_t xTestRegs;
	xHOS_MPU6050_factoryTrim_t xFT;

	/*	Check if accel full scale range is +-8g	*/
	ucSuccessful = ucHOS_MPU6050_readReg(pxHandle, MPU6050_REG_ACCEL_CONFIG, &ucReg);
	if (!ucSuccessful)
		return 0;

	if (((ucReg & 0b11000) >> 3) != 2)
		return 0;

	/*	Activate self test for all 3 accel units	*/
	ucSuccessful = ucEditReg(pxHandle, MPU6050_REG_ACCEL_CONFIG, 0b11100000, 0b11100000);
	if (!ucSuccessful)
		return 0;

	/*	Read outputs of all 3 accel units	*/
	ucSuccessful = ucReadAccelRawMeasurement(pxHandle, &xRawData[0]);
	if (!ucSuccessful)
		return 0;

	/*	De-activate self test for all 3 accel units	*/
	ucSuccessful = ucEditReg(pxHandle, MPU6050_REG_ACCEL_CONFIG, 0b11100000, 0);
	if (!ucSuccessful)
		return 0;

	/*	Read outputs of all 3 accel units	*/
	ucSuccessful = ucReadAccelRawMeasurement(pxHandle, &xRawData[1]);
	if (!ucSuccessful)
		return 0;

	/*	Calculate self test response and store it in "xRawData[0]"	*/
	xRawData[0].sX = xRawData[0].sX - xRawData[1].sX;
	xRawData[0].sY = xRawData[0].sY - xRawData[1].sY;
	xRawData[0].sZ = xRawData[0].sZ - xRawData[1].sZ;

	/*	Read accel self test registers	*/
	ucReadAccelSelfTestRegisters(pxHandle, &xTestRegs);

	/*	Calculate factory trim value	*/
	if (xTestRegs.ucX == 0)
		xFT.fX = 0;
	else
		xFT.fX = 4096.0f * 0.34f * (float)pow(0.92/0.34, ((float)(xTestRegs.ucX-1)) / 30.0f);

	if (xTestRegs.ucY == 0)
		xFT.fY = 0;
	else
		xFT.fY = 4096.0f * 0.34f * (float)pow(0.92/0.34, ((float)(xTestRegs.ucY-1)) / 30.0f);

	if (xTestRegs.ucZ == 0)
		xFT.fZ = 0;
	else
		xFT.fZ = 4096.0f * 0.34f * (float)pow(0.92/0.34, ((float)(xTestRegs.ucZ-1)) / 30.0f);

	/*
	 * calculate percentage change from factory trim, and decide whether gyro units
	 * pass the self test or not.
	 */
	pxResults->ucXPassed = fabs(((float)xRawData[0].sX - xFT.fX) / xFT.fX) < 14.0;
	pxResults->ucYPassed = fabs(((float)xRawData[0].sY - xFT.fY) / xFT.fY) < 14.0;
	pxResults->ucZPassed = fabs(((float)xRawData[0].sZ - xFT.fZ) / xFT.fZ) < 14.0;

	/*	No communication error occurred	*/
	return 1;
}

/*	See header for info	*/
uint8_t ucHOS_MPU6050_setGyroFullScaleRange(xHOS_MPU6050_t* pxHandle, uint8_t ucFSR)
{
	switch(ucFSR)
	{
	case 0:
		pxHandle->sGyroFSR = 250;
		break;

	case 1:
		pxHandle->sGyroFSR = 500;
		break;

	case 2:
		pxHandle->sGyroFSR = 1000;
		break;

	case 3:
		pxHandle->sGyroFSR = 2000;
		break;

	default:
		return 0;
	}

	uint8_t ucSuccesssful = ucEditReg(pxHandle, MPU6050_REG_GYRO_CONFIG, 0b11000, ucFSR << 3);
	if (!ucSuccesssful)
		return 0;

	return 1;
}

/*	See header for info	*/
uint8_t ucHOS_MPU6050_setAccelFullScaleRange(xHOS_MPU6050_t* pxHandle, uint8_t ucFSR)
{
	switch(ucFSR)
	{
	case 0:
		pxHandle->sAccelFSR = 2;
		break;

	case 1:
		pxHandle->sAccelFSR = 4;
		break;

	case 2:
		pxHandle->sAccelFSR = 8;
		break;

	case 3:
		pxHandle->sAccelFSR = 16;
		break;

	default:
		return 0;
	}

	uint8_t ucSuccesssful = ucEditReg(pxHandle, MPU6050_REG_ACCEL_CONFIG, 0b11000, ucFSR << 3);
	if (!ucSuccesssful)
		return 0;

	return 1;
}

/*	See header for info	*/
uint8_t ucHOS_MPU6050_confLpdfAndSampleRate(	xHOS_MPU6050_t* pxHandle,
												uint8_t ucLpdfConf,
												uint8_t ucDiv)
{
	uint8_t ucSuccesssful;

	/*	Write sample rate division	*/
	ucSuccesssful = ucHOS_MPU6050_writeReg(pxHandle, MPU6050_REG_SMPLRT_DIV, ucDiv);
	if (!ucSuccesssful)
		return 0;

	/*	Write LPDF configuration	*/
	if (ucLpdfConf > 7)
		return 0;

	ucSuccesssful = ucEditReg(pxHandle, MPU6050_REG_CONFIG, 0b111, ucLpdfConf);
	if (!ucSuccesssful)
		return 0;

	return 1;
}

/*	See header for info	*/
uint8_t ucHOS_MPU6050_readGyroMeasurement(	xHOS_MPU6050_t* pxHandle,
											xHOS_MPU6050_measurement_t* pxData	)
{
	uint8_t ucSuccessful;
	xHOS_MPU6050_rawMeasurement_t xRawMeasurement;

	/*	Read raw measurement	*/
	ucSuccessful = ucReadGyroRawMeasurement(pxHandle, &xRawMeasurement);
	if (!ucSuccessful)
		return 0;

	/*	Convert it to milli-dps value	*/
	pxData->iX = ((int32_t)xRawMeasurement.sX * (int32_t)(pxHandle->sGyroFSR) * 1000) / 32768;
	pxData->iY = ((int32_t)xRawMeasurement.sY * (int32_t)(pxHandle->sGyroFSR) * 1000) / 32768;
	pxData->iZ = ((int32_t)xRawMeasurement.sZ * (int32_t)(pxHandle->sGyroFSR) * 1000) / 32768;

	/*	Subtract drift error	*/
	pxData->iX -= pxHandle->xGyroDrift.iX;
	pxData->iY -= pxHandle->xGyroDrift.iY;
	pxData->iZ -= pxHandle->xGyroDrift.iZ;

	return 1;
}

/*	See header for info	*/
uint8_t ucHOS_MPU6050_readAccelMeasurement(	xHOS_MPU6050_t* pxHandle,
											xHOS_MPU6050_measurement_t* pxData	)
{
	uint8_t ucSuccessful;
	xHOS_MPU6050_rawMeasurement_t xRawMeasurement;

	/*	Read raw measurement	*/
	ucSuccessful = ucReadAccelRawMeasurement(pxHandle, &xRawMeasurement);
	if (!ucSuccessful)
		return 0;

	/*	Convert it to milli-g value	*/
	pxData->iX = ((int32_t)xRawMeasurement.sX * (int32_t)(pxHandle->sAccelFSR) * 1000) / 32768;
	pxData->iY = ((int32_t)xRawMeasurement.sY * (int32_t)(pxHandle->sAccelFSR) * 1000) / 32768;
	pxData->iZ = ((int32_t)xRawMeasurement.sZ * (int32_t)(pxHandle->sAccelFSR) * 1000) / 32768;

	/*	Subtract drift error	*/
	pxData->iX -= pxHandle->xAccelDrift.iX;
	pxData->iY -= pxHandle->xAccelDrift.iY;
	pxData->iZ -= pxHandle->xAccelDrift.iZ;

	return 1;
}

/*	See header for info	*/
uint8_t ucHOS_MPU6050_readTemperatureMeasurement(	xHOS_MPU6050_t* pxHandle,
													int32_t* piTemp	)
{
	uint8_t ucSuccessful;

	/*	Prepare transreceive params	*/
	uint8_t ucAddress = MPU6050_REG_TEMP_OUT_H;
	int16_t sTempRaw;
	xHOS_I2C_transreceiveParams_t xTRParams = {
		.ucUnitNumber = pxHandle->ucI2CUnitNumber,
		.usAddress = MPU6050_ADDRESS0 | pxHandle->ucAdd0State,
		.ucIs7BitAddress = 1,
		.pucTxArr = &ucAddress,
		.uiTxSize = 1,
		.pucRxArr = (uint8_t*)&sTempRaw,
		.uiRxSize = 2
	};

	/*	Brust read registers	*/
	ucSuccessful = ucHOS_I2C_masterTransReceive(&xTRParams);
	if (!ucSuccessful)
		return 0;

	/*	Swap bytes in variable	*/
	sTempRaw = ((sTempRaw & 0xFF) << 8) | ((sTempRaw & 0xFF00) >> 8);

	/*	Calculate temprature	*/
	*piTemp = (((int32_t)sTempRaw) * 1000) / 340 + 36530;

	return 1;
}














