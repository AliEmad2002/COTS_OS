/*
 * Port_RTC.h
 *
 *  Created on: Sep 16, 2023
 *      Author: Ali Emad
 */

/*	Target checking	*/
#include "MCAL_Port/Port_Target.h"
#ifdef MCAL_PORT_TARGET_STM32F401RCT6


#ifndef COTS_OS_INC_MCAL_PORT_PORT_RTC_H_
#define COTS_OS_INC_MCAL_PORT_PORT_RTC_H_

#include "stm32f4xx_ll_rtc.h"

/*******************************************************************************
 * Structures:
 ******************************************************************************/
/*
 * Human-readable timestamp representation.
 * (All values are absolute and are not related to base time or any other time)
 */
typedef struct{
	uint16_t usYear;
	uint16_t usMilliSecond;
	uint8_t ucMonth;
	uint8_t ucDay;
	uint8_t ucHour;
	uint8_t ucMinute;
	uint8_t ucSecond;
}xMCAL_RTC_Time_t;

/*******************************************************************************
 * API functions:
 ******************************************************************************/
/*
 * Given a handle of "xMCAL_RTC_Time_t", this function returns total number of
 * seconds since the starting time configured above.
 */
uint32_t uiPort_RTC_convertTimeToTotalSeconds(const xMCAL_RTC_Time_t* pxTime);

/*
 * Given total number of seconds passed since
 */
void vPort_RTC_convertTotalSecondsToTime(	uint32_t uiTotalSeconds,
											xMCAL_RTC_Time_t* pxTime	);

/*
 * Initializes RTC.
 */
void vPort_RTC_init(void);

/*
 * Changes RTC time count.
 */
void vPort_RTC_setTime(const xMCAL_RTC_Time_t* pxTime);

/*
 * Enables (Runs) RTC.
 */
void vPort_RTC_enable(void);

/*
 * Disables (Runs) RTC.
 */
void vPort_RTC_disable(void);

/*
 * Gets current time.
 */
void vPort_RTC_getTime(xMCAL_RTC_Time_t* pxTime);

/*
 * Gets current time in a string.
 *
 * Notes:
 * 		-	Format: yyyy/mm/dd hr:min:sec:ms
 * 			(e.g: 2002/3/21 18:59:55:82).
 *
 * 		-	Hence "pcStr" must be of at least 24 byte.
 */
void vPort_RTC_getTimeStr(char* pcStr);

///*
// * Gets difference between two timestamps.
// *
// * Notes:
// * 		-	TimeDif = Time2 - Time1.
// * 		-	Time2 must be larger than Time1.
// */
//void vPort_RTC_getTimestampDif(	const xMCAL_RTC_TimeAccurate_t* pxTime1,
//								const xMCAL_RTC_TimeAccurate_t* pxTime2,
//								xMCAL_RTC_TimeAccurate_t* pxTimeDif	);

/*
 * Enables RTC calibration output.
 */
void vPort_RTC_enableCalibrationOutput(void);










#endif /* COTS_OS_INC_MCAL_PORT_PORT_RTC_H_ */


#endif /* Target checking */
