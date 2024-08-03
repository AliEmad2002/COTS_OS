/*
 * Port_RTC.c
 *
 *  Created on: Sep 16, 2023
 *      Author: Ali Emad
 */

/*	Target checking	*/
#include "MCAL_Port/Port_Target.h"
#ifdef MCAL_PORT_TARGET_STM32F401RCT6


#include <stdint.h>
#include <time.h>

#include "stm32f4xx.h"
#include "stm32f4xx_ll_pwr.h"
#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_hal_rtc.h"

#include "MCAL_Port/Port_BKP.h"
#include "MCAL_Port/Port_RTC.h"

#include "FreeRTOS.h"
#include "task.h"

/*******************************************************************************
 * Helping functions / macros:
 ******************************************************************************/
#define uiRTCCLK	32768	//32000

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
	return 0; // TODO
}

/*
 * See header for info.
 */
void vPort_RTC_convertTotalSecondsToTime(	uint32_t uiTotalSeconds,
											xMCAL_RTC_Time_t* pxTime	)
{
	// TODO
}

/*
 * See header for info.
 */
void inline vPort_RTC_init(void)
{
	RTC_HandleTypeDef xRTC;
	xRTC.Instance = RTC;
	xRTC.Init.HourFormat = RTC_HOURFORMAT_24;
	xRTC.Init.AsynchPrediv = 0x7F;
	xRTC.Init.SynchPrediv = 0xFF;
	xRTC.Init.OutPut = RTC_OUTPUT_DISABLE;
	xRTC.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
	xRTC.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
	HAL_RTC_Init(&xRTC);

	/*	Shadow registers are bypassed based on errata's advice: ES0222-Rev7, 2.7.2	*/
	vPORT_BKP_UNLOCK_DATA_REGISTERS();
	LL_RTC_EnableShadowRegBypass(RTC);
	vPORT_BKP_LOCK_DATA_REGISTERS();
}

/*
 * See header for info.
 */
void vPort_RTC_setTime(const xMCAL_RTC_Time_t* pxTime)
{
//	taskENTER_CRITICAL();

	RTC_HandleTypeDef xRTC;
	xRTC.Instance = RTC;

	RTC_TimeTypeDef xTime;
	xTime.Hours = pxTime->ucHour;
	xTime.Minutes = pxTime->ucMinute;
	xTime.Seconds = pxTime->ucSecond;
	xTime.TimeFormat = 0;

	RTC_DateTypeDef xDate;
	xDate.Year = pxTime->usYear - 2000;
	xDate.Month = pxTime->ucMonth;
	xDate.Date = pxTime->ucDay;
	xDate.WeekDay = 0;

	HAL_RTC_SetTime(&xRTC, &xTime, RTC_FORMAT_BIN);
	HAL_RTC_SetDate(&xRTC, &xDate, RTC_FORMAT_BIN);


//	taskEXIT_CRITICAL();
}

/*
 * See header for info.
 */
void vPort_RTC_enable(void)
{
	/*	Unlock backup domain	*/
	vPORT_BKP_UNLOCK_DATA_REGISTERS();

	/*	Enable	*/
	__HAL_RCC_RTC_ENABLE();

	/*	Lock backup domain	*/
	vPORT_BKP_LOCK_DATA_REGISTERS();
}

/*
 * See header for info.
 */
void vPort_RTC_disable(void)
{
	/*	Unlock backup domain	*/
	vPORT_BKP_UNLOCK_DATA_REGISTERS();

	/*	Disable	*/
	__HAL_RCC_RTC_DISABLE();

	/*	Lock backup domain	*/
	vPORT_BKP_LOCK_DATA_REGISTERS();
}

/*
 * See header for info.
 */
void vPort_RTC_getTime(xMCAL_RTC_Time_t* pxTime)
{
	RTC_HandleTypeDef xRTC;
	xRTC.Instance = RTC;

	RTC_TimeTypeDef xTime;
	RTC_DateTypeDef xDate;

	HAL_RTC_GetTime(&xRTC, &xTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&xRTC, &xDate, RTC_FORMAT_BIN);

	pxTime->ucHour = xTime.Hours;
	pxTime->ucMinute = xTime.Minutes;
	pxTime->ucSecond = xTime.Seconds;
	pxTime->usMilliSecond = (1000 * (0xFF - xTime.SubSeconds)) / (0xFF + 1);

	pxTime->usYear = xDate.Year + 2000;
	pxTime->ucMonth = xDate.Month;
	pxTime->ucDay = xDate.Date;
}

/*
 * See header for info.
 */
void vPort_RTC_getTimeStr(char* pcStr)
{
	xMCAL_RTC_Time_t xTime;
	vPort_RTC_getTime(&xTime);

	/*	Print data on the string	*/
	sprintf(
		pcStr,
		"%d/%d/%d %d:%d:%d:%d",
		(int)xTime.usYear,
		(int)xTime.ucMonth,
		(int)xTime.ucDay,
		(int)xTime.ucHour,
		(int)xTime.ucMinute,
		(int)xTime.ucSecond,
		(int)xTime.usMilliSecond	);
}

///*
// * See header for info.
// */
//void vPort_RTC_getTimestampDif(	const xMCAL_RTC_TimeAccurate_t* pxTime1,
//								const xMCAL_RTC_TimeAccurate_t* pxTime2,
//								xMCAL_RTC_TimeAccurate_t* pxTimeDif	)
//{
//
//}

/*
 * See header for info.
 */
void vPort_RTC_enableCalibrationOutput(void)
{
	/*	Unlock backup domain	*/
	vPORT_BKP_UNLOCK_DATA_REGISTERS();

	/*	Configure output	*/
//	LL_RTC_SetOutputSource(BKP, LL_RTC_CALIB_OUTPUT_SECOND);

	/*	Lock backup domain	*/
	vPORT_BKP_LOCK_DATA_REGISTERS();
}











#endif /* Target checking */
