/*
 * MPU6050.h
 *
 *  Created on: Jul 19, 2023
 *      Author: Ali Emad
 */

#ifndef COTS_OS_INC_HAL_MPU6050_MPU6050_H_
#define COTS_OS_INC_HAL_MPU6050_MPU6050_H_

/*	Manufacturing error drift. Used units are milli-g or milli-dps	*/
typedef struct{
	int32_t iX;
	int32_t iY;
	int32_t iZ;
}xHOS_MPU6050_drift_t;

/*	Measurement in milli	*/
typedef struct{
	int32_t iX;
	int32_t iY;
	int32_t iZ;
}xHOS_MPU6050_measurement_t;

/*
 * Module structure.
 * Please read descriptions of all variables. Trivial variable names will not be
 * described.
 */
typedef struct{
	/*	PUBLIC	*/
	uint8_t ucI2CUnitNumber;

	/*	State of the AD0 pin	*/
	uint8_t ucAdd0State : 1;

	/*	States whether INT pin is connected or not	*/
	uint8_t ucIsIntConnected : 1;

	/*
	 * DIO pin connected to MPU6050's INT pin.
	 * (Used only when tilt calculation is enabled, otherwise it is ignored).
	 */
	uint8_t ucIntPort : 4;
	uint8_t ucIntPin  : 4;

	/*	Gyro manufacturing error. in milli-dps	*/
	xHOS_MPU6050_drift_t xGyroDrift;

	/*	Accel manufacturing error. in mg	*/
	xHOS_MPU6050_drift_t xAccelDrift;

	/*
	 * Tilt angles in milli-degrees.
	 * (Read only, and valid only when tilt calculation is enabled)
	 */
	xHOS_MPU6050_measurement_t xTilt;

	/*	PRIVATE	*/
	int16_t sGyroFSR;
	int16_t sAccelFSR;

	xHOS_MPU6050_measurement_t xPrevGyroMeasurement;

	StackType_t puxTaskStack[configMINIMAL_STACK_SIZE];
	StaticTask_t xTaskStatic;
	TaskHandle_t xTask;
}xHOS_MPU6050_t;

typedef struct{
	uint8_t ucXPassed : 1;
	uint8_t ucYPassed : 1;
	uint8_t ucZPassed : 1;
}xHOS_MPU6050_testResult_t;


/*
 * Reads register given its address. Used for debugging.
 */
uint8_t ucHOS_MPU6050_readReg(xHOS_MPU6050_t* pxHandle, uint8_t ucAddress, uint8_t* pucReg);

/*
 * Writes register given its address. Used for debugging.
 */
uint8_t ucHOS_MPU6050_writeReg(xHOS_MPU6050_t* pxHandle, uint8_t ucAddress, uint8_t ucReg);

/*
 * Initializes MPU6050 object.
 *
 * Notes:
 * 		-	This function must be called before scheduler start.
 * 		-	All "PUBLIC" elements of the handle must be initialized first.
 */
void vHOS_MPU6050_init(xHOS_MPU6050_t* pxHandle);

/*
 * Enables MPU6050 object.
 *
 * Notes:
 * 		-	This function must be called after scheduler start.
 * 		-	MPU6050 object is initially in sleep mode.
 */
void vHOS_MPU6050_enable(xHOS_MPU6050_t* pxHandle);

/*
 * Disables (sleep mode) MPU6050 object.
 *
 * Notes:
 * 		-	This function must be called after scheduler start.
 * 		-	MPU6050 object is initially in sleep mode.
 */
void vHOS_MPU6050_disable(xHOS_MPU6050_t* pxHandle);

/*
 * Enables tilt calculation.
 * Notes:
 * 		-	This function must be called after scheduler start.
 * 		-	Sample rate must be first set to 1kHz.
 * 		-	INT pin must be connected and "ucIntPort", "ucIntPin" must be initialized.
 * 		-	Tilt calculation is initially disabled.
 */
uint8_t ucHOS_MPU6050_enableTiltCalculation(xHOS_MPU6050_t* pxHandle);

/*
 * Disables tilt calculation.
 * Notes:
 * 		-	This function must be called after scheduler start.
 * 		-	Tilt calculation is initially disabled.
 */
void vHOS_MPU6050_disableTiltCalculation(xHOS_MPU6050_t* pxHandle);

/*
 * Enters debugger mode, which gives user chance to easily read / write registers
 * of the MPU6050 using OCD.
 *
 * Notes:
 * 		-	This function must be called after scheduler start.
 */
void vHOS_MPU6050_enterDebuggerMode(xHOS_MPU6050_t* pxHandle);

/*
 * Runs self test for all 3 gyro units.
 *
 * Notes:
 * 		-	Results of this function are valid only when gyro full-scale range
 * 			is +-250dps (degree per second), which is the reset value. Thus,
 * 			this function better be called before configuring any thing in the
 * 			MPU6050.
 */
uint8_t ucHOS_MPU6050_gyroSelfTest(	xHOS_MPU6050_t* pxHandle,
									xHOS_MPU6050_testResult_t* pxResults	);

/*
 * Runs self test for all 3 accel units.
 *
 * Notes:
 * 		-	Results of this function are valid only when accel full-scale range
 * 			is +-8g, which is **NOT** the reset value. Thus, setting full scale range
 * 			before calling this function is a must.
 */
uint8_t ucHOS_MPU6050_accelSelfTest(	xHOS_MPU6050_t* pxHandle,
									xHOS_MPU6050_testResult_t* pxResults	);

/*
 * Sets gyro full scale range.
 *
 * Notes:
 * 		-	"ucFSR"		full scale range in dps (degree per second).
 * 			   0			+-250
 * 			   1			+-500
 * 			   2			+-1000
 * 			   3			+-2000
 */
uint8_t ucHOS_MPU6050_setGyroFullScaleRange(xHOS_MPU6050_t* pxHandle, uint8_t ucFSR);

/*
 * Sets accel full scale range.
 *
 * Notes:
 * 		-	"ucFSR"		full scale range in g (9.8 m/s^2).
 * 			   0			+-2
 * 			   1			+-4
 * 			   2			+-8
 * 			   3			+-16
 */
uint8_t ucHOS_MPU6050_setAccelFullScaleRange(xHOS_MPU6050_t* pxHandle, uint8_t ucFSR);

/*
 * Configures LPDF(low pass digital filter) and sampling rate.
 *
 * Notice:
 * 		-	Sample Rate = Gyroscope Output Rate / (1 + ucDiv),
 * 			where Gyroscope Output Rate = 8kHz when the DLPF is disabled, and 1kHz
 * 			when the DLPF is enabled.
 *
 *  	-	The accelerometer output rate is 1kHz.  This means that for a Sample
 *  		Rate greater than 1kHz, the same accelerometer sample may be output
 *  		more than once.
 *
 *  	-	"ucLpdfConf" is in the range from 0 to 7. Please refer to the table
 *			in page-17 of the document: "MPU-6000/MPU-6050 Register Map and Descriptions"
 *			for understanding what DLPF configuration each value gives.
 */
uint8_t ucHOS_MPU6050_confLpdfAndSampleRate(	xHOS_MPU6050_t* pxHandle,
												uint8_t ucLpdfConf,
												uint8_t ucDiv);

/*
 * TODO: add power management and low power mode
 */


/*
 * Reads gyro measurement (Angular velocity).
 *
 * Results are in milli-degree per second
 */
uint8_t ucHOS_MPU6050_readGyroMeasurement(	xHOS_MPU6050_t* pxHandle,
											xHOS_MPU6050_measurement_t* pxData	);

/*
 * Reads tilt measurement. (only when tilt calculation is enabled).
 *
 * Results are in milli-degree.
 */
uint8_t ucHOS_MPU6050_readTiltMeasurement(	xHOS_MPU6050_t* pxHandle,
											xHOS_MPU6050_measurement_t* pxData	);

/*
 * Reads accel measurement.
 *
 * Results are in milli-g (i.e.: 9.8 mm/s^2).
 */
uint8_t ucHOS_MPU6050_readAccelMeasurement(	xHOS_MPU6050_t* pxHandle,
											xHOS_MPU6050_measurement_t* pxData	);

/*
 * Reads temperature measurement.
 *
 * Results are in milli-deggree C.
 */
uint8_t ucHOS_MPU6050_readTemperatureMeasurement(	xHOS_MPU6050_t* pxHandle,
													int32_t* piTemp	);









#endif /* COTS_OS_INC_HAL_MPU6050_MPU6050_H_ */
