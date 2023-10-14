/*
 * DCMotor.h
 *
 *  Created on: Aug 31, 2023
 *      Author: Ali Emad
 *
 * This driver targets the L298 hardware.
 */

#ifndef COTS_OS_INC_HAL_DCMOTOR_DCMOTOR_H_
#define COTS_OS_INC_HAL_DCMOTOR_DCMOTOR_H_

typedef struct{
	/*		PUBLIC		*/
	/*	Enable pin.	*/
	uint8_t ucEnPort;
	uint8_t ucEnPin;

	/*	Direction control pin1 (PWM pin)	*/
	uint8_t ucDir1TimerUnitNumber;
	uint8_t ucDir1TimerChannelNumber;

	uint8_t ucDir2Port;
	uint8_t ucDir2Pin;
}xHOS_DCMotor_t;


/*
 * Initializes handle.
 *
 * Notes:
 * 		-	All public parameters of the passed handle must be previously initialized
 * 			with valid values.
 *
 * 		-	Timer unit frequency must be previously configured and initialized.
 *
 * 		-	Must be called before scheduler start.
 */
void vHOS_DCMotor_init(xHOS_DCMotor_t* pxHandle);

/*	Enables / Disables motor	*/
#define vHOS_DCMOTOR_ENABLE(pxHandle)	\
	(	vPORT_DIO_WRITE_PIN((pxHandle)->ucEnPort, (pxHandle)->ucEnPin, 1)	)

#define vHOS_DCMOTOR_DISABLE(pxHandle)	\
	(	vPORT_DIO_WRITE_PIN((pxHandle)->ucEnPort, (pxHandle)->ucEnPin, 0)	)

/*
 * Sets speed.
 *
 * Notes:
 * 		-	"iSpeed": is a value between -65535 and 65535.
 */
void vHOS_DCMotor_setSpeed(xHOS_DCMotor_t* pxHandle, int32_t iSpeed);




#endif /* COTS_OS_INC_HAL_DCMOTOR_DCMOTOR_H_ */
