/*
 * LowPassFilter.h
 *
 *  Created on: Jan 5, 2024
 *      Author: Ali Emad
 */

#ifndef COTS_OS_INC_LIB_LOWPASSFILTER_LOWPASSFILTER_H_
#define COTS_OS_INC_LIB_LOWPASSFILTER_LOWPASSFILTER_H_

#define uiHOS_LPF_STACK_SZ		(128)

typedef struct{
	/*		PUBLIC		*/
	/*	Filter's output (Read only)	*/
	float fOutput;

	/*	Sample getter callback	*/
	float(*pfGetSample)(void);

	/*	Sampling periodic time (in ms)	*/
	uint32_t uiSampleTimeMs;

	/*		PRIVATE		*/
	/*	Filter's constants	*/
	float pfC[2];

	/*	Previous sample	*/
	float fXPrev;
	float fYPrev;

	/*	Task	*/
	TaskHandle_t xTask;
	StaticTask_t xTaskStatic;
	StackType_t pxTaskStack[uiHOS_LPF_STACK_SZ];
}xLIB_LowPassFilter_t;

/*
 * Initializes handle.
 *
 * Notes:
 * 		-	All public parameters must be initialized first.
 * 		-	"fWc": Critical cutting frequency (in rad/sec).
 * 		-	"fK": filter gain.
 */
void vLIB_LowPassFilter_init(	xLIB_LowPassFilter_t* pxHandle,
								float fWc,
								float fK	);



#endif /* COTS_OS_INC_LIB_LOWPASSFILTER_LOWPASSFILTER_H_ */
