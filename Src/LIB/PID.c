 /* 
 * File:   PID.c
 * Author: ali20
 *
 * Created on March 21, 2024, 12:28 AM
 */

/*  CONF    */
#include "configure.h"

/*	LIB	*/
#include <stdint.h>
#include <stdio.h>

/*	SELF	*/
#include "PID.h"

/*******************************************************************************
 * Helping functions/macros:
 ******************************************************************************/
#define vCLAMP(fVal, fMin, fMax)     \
{                                    \
	if ((fVal) < (fMin))             \
		(fVal) = (fMin);             \
	else if ((fVal) > (fMax))        \
		(fVal) = (fMax);             \
}


/*******************************************************************************
 * API functions
 ******************************************************************************/
void PID_init(PID_t* pid)
{
	/*	Initialize handle variables	*/
	pid->I = 0;
	pid->E = 0;
    pid->setPoint = 0;
}

PID_Base_Type PID_update(PID_t* pid, PID_Base_Type feedback)
{
	/*	Get new error value	*/
	PID_Base_Type eNew = pid->setPoint - feedback;

	/*	If new error is below minimum error margin, make it zero	*/
	if (abs(eNew) < pid->eMin)
		eNew = 0;

	/*	calculate integral term (Multiplied by 1000), and clamp it	*/
	PID_Base_Type c0 = pid->ki * pid->updateTimePeriodMs;
	pid->I = pid->I + (c0 * (pid->E + eNew)) / 2000;
	vCLAMP(pid->I, pid->iMin, pid->iMax);

    /*	calculate derivative term (Multiplied by 1000), and clamp it	*/
    PID_Base_Type d = (pid->kd * 1000 * (eNew - pid->E)) / pid->updateTimePeriodMs;
    vCLAMP(d, pid->dMin, pid->dMax);
    
	/*	calculate proportional term (Multiplied by 1000)	*/
	PID_Base_Type p = pid->kp * eNew;

	/*	Sum  and clamp summation	*/
	PID_Base_Type sum = p + pid->I + d;
	vCLAMP(sum, pid->outMin, pid->outMax);
    
	/*	update controller variables	*/
	pid->E = eNew;

	/*	update plant with the new sum value	*/
	return sum / 1000;
}