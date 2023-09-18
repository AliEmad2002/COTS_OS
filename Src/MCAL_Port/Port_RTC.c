/*
 * Port_RTC.c
 *
 *  Created on: Sep 16, 2023
 *      Author: Ali Emad
 */

#include <stdint.h>

#include "stm32f1xx.h"
#include "stm32f1xx_ll_pwr.h"

#include "MCAL_Port/Port_BKP.h"
#include "MCAL_Port/Port_RTC.h"


/*******************************************************************************
 * Helping functions / macros:
 ******************************************************************************/
#define uiRTCCLK	32768

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
 * See header for info.
 */
uint32_t uiPort_RTC_convertTimeToTotalSeconds(const xMCAL_RTC_Time_t* pxTime)
{

}

/*
 * See header for info.
 */
void vPort_RTC_convertTotalSecondsToTime(	uint32_t uiTotalSeconds,
											xMCAL_RTC_Time_t* pxTime	)
{

}

/*
 * See header for info.
 */
void inline vPort_init(void)
{
	/*	Unlock backup domain	*/
	LL_PWR_EnableBkUpAccess();

	/*	Configure RTC clock	*/
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE;
	RCC_OscInitStruct.LSEState = RCC_LSE_ON;
	vLib_ASSERT(HAL_RCC_OscConfig(&RCC_OscInitStruct) == HAL_OK, 0);

	__HAL_RCC_RTC_CONFIG(RCC_RTCCLKSOURCE_LSE);
	__HAL_RCC_RTC_ENABLE();

	/*
	 * As recommended in RFS bit description, it must be cleared on startup, and
	 * waited until it becomes set.
	 */
	LL_RTC_ClearFlag_RS(RTC);
	while(!LL_RTC_IsActiveFlag_RS(RTC));

	/*	Following the configuration procedure described in RM0008 page485	*/

	/*	Poll RTOFF, ait until its value goes to 1	*/
	while(!LL_RTC_IsActiveFlag_RTOF(RTC));

	/*	Set the CNF bit to enter configuration mode	*/
	LL_RTC_DisableWriteProtection(RTC);

	/*	Change RTC clock to the calibrated value (This value is experimental)	*/
	LL_RTC_CAL_SetCoarseDigital(BKP, 67);

	/*	Write RTC registers	*/
	/*	Since: fTR_CLK = fRTCCLK/(PRL+1) ==> PRL = fRTCCLK / fTR_CLK - 1	*/
	LL_RTC_SetAsynchPrescaler(RTC, uiRTCCLK / 1 - 1);

	/*	Clear the CNF bit to exit configuration mode	*/
	LL_RTC_EnableWriteProtection(RTC);

	/*	Poll RTOFF, wait until its value goes to ‘1’ to check the end of the write operation.	*/
	while(!LL_RTC_IsActiveFlag_RTOF(RTC));

	/*	Lock backup domain	*/
	LL_PWR_DisableBkUpAccess();
}

/*
 * See header for info.
 */
void vPort_setTime(uint32_t uiCurrentTotalSeconds)
{
	/*	Following the configuration procedure described in RM0008 page485	*/

	/*	Poll RTOFF, ait until its value goes to 1	*/
	while(!LL_RTC_IsActiveFlag_RTOF(RTC));

	/*	Set the CNF bit to enter configuration mode	*/
	LL_RTC_DisableWriteProtection(RTC);

	/*	Write RTC registers	*/
	LL_RTC_TIME_Set(RTC, uiCurrentTotalSeconds);

	/*	Clear the CNF bit to exit configuration mode	*/
	LL_RTC_EnableWriteProtection(RTC);

	/*	Poll RTOFF, wait until its value goes to ‘1’ to check the end of the write operation.	*/
	while(!LL_RTC_IsActiveFlag_RTOF(RTC));
}

/*
 * See header for info.
 */
void vPort_RTC_enable(void)
{
	/*	Unlock backup domain	*/
	LL_PWR_EnableBkUpAccess();

	/*	Enable	*/
	__HAL_RCC_RTC_ENABLE();

	/*	Lock backup domain	*/
	LL_PWR_DisableBkUpAccess();
}

/*
 * See header for info.
 */
void vPort_RTC_disable(void)
{
	/*	Unlock backup domain	*/
	LL_PWR_EnableBkUpAccess();

	/*	Disable	*/
	__HAL_RCC_RTC_DISABLE();

	/*	Lock backup domain	*/
	LL_PWR_DisableBkUpAccess();
}

/*
 * See header for info.
 */
void vPort_RTC_getTimestamp(xMCAL_RTC_TimeAccurate_t* pxTime)
{
	pxTime->uiTime = LL_RTC_TIME_Get(RTC);
	pxTime->uiMicroSeconds = (LL_RTC_GetDivider(RTC) * 1000000) / uiRTCCLK;
}

/*
 * See header for info.
 */
void vPort_RTC_getTimestampDif(	const xMCAL_RTC_TimeAccurate_t* pxTime1,
								const xMCAL_RTC_TimeAccurate_t* pxTime2,
								xMCAL_RTC_TimeAccurate_t* pxTimeDif	)
{

}

/*
 * See header for info.
 */
void vPort_RTC_enableCalibrationOutput(void)
{
	/*	Unlock backup domain	*/
	LL_PWR_EnableBkUpAccess();

	/*	Configure output	*/
	LL_RTC_SetOutputSource(BKP, LL_RTC_CALIB_OUTPUT_SECOND);

	/*	Lock backup domain	*/
	LL_PWR_DisableBkUpAccess();
}









