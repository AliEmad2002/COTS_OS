/*
 * Port_Clock.h
 *
 *  Created on: Jun 14, 2023
 *      Author: Ali Emad
 */

#ifndef HAL_OS_PORT_PORT_CLOCK_H_
#define HAL_OS_PORT_PORT_CLOCK_H_

#include "stm32f103xb.h"
#include "stm32f1xx_hal.h"
#include "LIB/Assert.h"

#define uiPORT_CLOCK_MAIN_HZ		(SystemCoreClock)

/*	Used with STM32 targets only	*/
#define uiPORT_CLOCK_AHB_DIV	RCC_SYSCLK_DIV1
#define uiPORT_CLOCK_APB1_DIV	RCC_HCLK_DIV2
#define uiPORT_CLOCK_APB2_DIV	RCC_SYSCLK_DIV1

/*
 * Initializes clock control peripheral, and enables the needed peripherals.
 * If the ported target does not have any, or has implementation of clock control
 * defined privately and called at startup, this wrapper is left empty.
 */
static inline void vPort_Clock_init(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

	/** Initializes the RCC Oscillators according to the specified parameters
	* in the RCC_OscInitTypeDef structure.
	*/
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
	vLib_ASSERT(HAL_RCC_OscConfig(&RCC_OscInitStruct) == HAL_OK, 0);

	/** Initializes the CPU, AHB and APB buses clocks
	*/
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
								  |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = uiPORT_CLOCK_AHB_DIV;
	RCC_ClkInitStruct.APB1CLKDivider = uiPORT_CLOCK_APB1_DIV;
	RCC_ClkInitStruct.APB2CLKDivider = uiPORT_CLOCK_APB2_DIV;

	vLib_ASSERT(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) == HAL_OK, 0);

	SystemCoreClockUpdate();

	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_AFIO_CLK_ENABLE();

	__HAL_RCC_SPI1_CLK_ENABLE();

	__HAL_RCC_I2C1_CLK_ENABLE();

	__HAL_RCC_TIM1_CLK_ENABLE();
	__HAL_RCC_TIM2_CLK_ENABLE();
	__HAL_RCC_TIM3_CLK_ENABLE();
	__HAL_RCC_TIM4_CLK_ENABLE();

	__HAL_RCC_DMA1_CLK_ENABLE();


}



#endif /* HAL_OS_PORT_PORT_CLOCK_H_ */
