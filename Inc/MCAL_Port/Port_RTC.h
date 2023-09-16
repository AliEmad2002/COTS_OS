/*
 * Port_RTC.h
 *
 *  Created on: Sep 16, 2023
 *      Author: Ali Emad
 */

#ifndef COTS_OS_INC_MCAL_PORT_PORT_RTC_H_
#define COTS_OS_INC_MCAL_PORT_PORT_RTC_H_


/*******************************************************************************
 * Structures:
 ******************************************************************************/
/*
 * Human-readable timestamp representation.
 * (All values are absolute and are not related to base time or any other time)
 */
typedef struct{
	uint16_t usYear;
	uint8_t ucMonth;
	uint8_t ucDay;
	uint8_t ucHour;
	uint8_t ucMinute;
	uint8_t ucSecond;
}xMCAL_RTC_Time_t;

/*
 * Accurate timestamp representation.
 */
typedef struct{
	uint32_t uiTime;	/*	Number of seconds since the configured starting time	*/
	uint32_t uiMicroSeconds;/*	Number of seconds passed since the beginning of the last second	*/
}xMCAL_RTC_TimeAccurate_t;

/*******************************************************************************
 * Base time:
 *
 * Since the driver is using an unsigned 32-bit value to represent total number
 * of seconds since the  base time, this value has a maximum of about 139 years.
 * Hence, the "xPortRtcStartingTime" must not be too old.
 ******************************************************************************/
const xMCAL_RTC_Time_t xPortRtcStartingTime = {
	.usYear = 1970,
	.ucMonth = 1,
	.ucDay = 1,
	.ucHour = 0,
	.ucMinute = 0,
	.ucSecond = 0
};

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
 * Initializes RTC with new time settings.
 *
 * Note: RTC must be disabled before changing time.
 */
void inline vPort_init(uint32_t uiCurrentTotalSeconds);

/*
 * Enables (Runs) RTC.
 */
void inline vPort_RTC_enable(void)
{

}

/*
 * Disables (Runs) RTC.
 */
void inline vPort_RTC_disable(void)
{

}

/*
 * Gets current timestamp.
 */
void vPort_RTC_getTimestamp(xMCAL_RTC_TimeAccurate_t* pxTime);

/*
 * Gets difference between two timestamps.
 *
 * Notes:
 * 		-	TimeDif = Time2 - Time1.
 * 		-	Time2 must be larger than Time1.
 */
void vPort_RTC_getTimestampDif(	const xMCAL_RTC_TimeAccurate_t* pxTime1,
								const xMCAL_RTC_TimeAccurate_t* pxTime2,
								xMCAL_RTC_TimeAccurate_t* pxTimeDif	);












#endif /* COTS_OS_INC_MCAL_PORT_PORT_RTC_H_ */
