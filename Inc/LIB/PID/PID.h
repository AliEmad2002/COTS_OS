 /* 
 * File:   PID.h
 * Author: ali20
 *
 * Created on March 21, 2024, 12:28 AM
 */

#ifndef PID_H_
#define PID_H_

/*******************************************************************************
 * Helping macros / preprocessors:
 ******************************************************************************/
typedef int32_t PID_Base_Type;
 
 /*******************************************************************************
 * Structures:
 ******************************************************************************/
typedef struct{
	/*		PRIVATE		*/
	PID_Base_Type E;	// Previous error.
	PID_Base_Type I;	// Integral term multiplied by 1000.


	/*		PUBLIC 		*/
	/*
	 * Any of the following variables can be changed while handle is enabled and
	 * running, and effects will take place next update. They must be initialized
	 * with valid values before enabling the controller.
	 */
	uint8_t updateTimePeriodMs;
	
	PID_Base_Type setPoint;

    PID_Base_Type kp;	//	kp multiplied by 1000
	PID_Base_Type ki;	//	ki multiplied by 1000
    PID_Base_Type kd;	//	kp multiplied by 1000

	PID_Base_Type iMax;	//	maximum value of accumulated integral term, multiplied by 1000
	PID_Base_Type iMin;	//	minimum value of accumulated integral term, multiplied by 1000

    PID_Base_Type dMax;	//	maximum value of derivative term, multiplied by 1000
    PID_Base_Type dMin;	//	minimum value of derivative term, multiplied by 1000
    
	PID_Base_Type outMax;	//	maximum value of controller's output, multiplied by 1000
	PID_Base_Type outMin;	//	minimum value of controller's output, multiplied by 1000
    
    PID_Base_Type eMin; // Minimum error in order for the compensator to work
}PID_t;

/*
 * Initializes PID controller.
 */
void PID_init(PID_t* pid);

/*
 * Updates controller.
 *
 * Notes:
 *		-	This function should be called at a constant rate of "pid->ucUpdateTimePeriodMs".
 *
 *		-	Return value of this function is the value to be applied on the controlled quantity.
 */ 
PID_Base_Type PID_update(PID_t* pid, PID_Base_Type feedback);

#endif /* PID_H_ */
