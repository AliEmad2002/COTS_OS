/*
 * Port_Clock.c
 *
 *  Created on: Sep 25, 2023
 *      Author: Ali Emad
 */

/*	Target checking	*/
#include "MCAL_Port/Port_Target.h"
#ifdef MCAL_PORT_TARGET_STM32F401RCT6

/*	LIB	*/
#include "stdint.h"

/*	MCAL	*/
#include "stm32f4xx.h"

#include "MCAL_Port/Port_Clock.h"

void vPort_Clock_initCpuClock(void)
{
	  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

	  /*
	   * Prepare flash peripheral.
	   */
	  __HAL_FLASH_INSTRUCTION_CACHE_ENABLE();
	  __HAL_FLASH_DATA_CACHE_ENABLE();
	  __HAL_FLASH_PREFETCH_BUFFER_ENABLE();

	  /** Configure the main internal regulator output voltage
	  */
	  __HAL_RCC_PWR_CLK_ENABLE();
	  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

	  /** Initializes the RCC Oscillators according to the specified parameters
	  * in the RCC_OscInitTypeDef structure.
	  */
	  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_LSI;
	  RCC_OscInitStruct.LSEState = RCC_LSE_OFF;
	  RCC_OscInitStruct.LSIState = RCC_LSI_OFF;
	  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	  RCC_OscInitStruct.PLL.PLLM = 25;
	  RCC_OscInitStruct.PLL.PLLN = 336;
	  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
	  RCC_OscInitStruct.PLL.PLLQ = 7;
	  vLib_ASSERT(HAL_RCC_OscConfig(&RCC_OscInitStruct) == HAL_OK, 0);

	  /** Initializes the CPU, AHB and APB buses clocks
	  */
	  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
	                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
	  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	  vLib_ASSERT(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) == HAL_OK, 0);
}

void vPort_Clock_initPeriphClock(void)
{
	/*	Initialize GPIO	clock	*/
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();

	/*	Initialize SYSCFG clock	*/
	__HAL_RCC_SYSCFG_CLK_ENABLE();

	/*	Initialize PWR clock	*/
	__HAL_RCC_PWR_CLK_ENABLE();

//	/*	Initialize RTC clock	*/
//	__HAL_RCC_RTC_CONFIG(RCC_RTCCLKSOURCE_LSE);
//	__HAL_RCC_RTC_ENABLE();

	/*	Initialize ADC clock	*/
	__HAL_RCC_ADC1_CLK_ENABLE();

//	/*	Initialize UART clock	*/
//	__HAL_RCC_USART1_CLK_ENABLE();
//	__HAL_RCC_USART2_CLK_ENABLE();

//	/*	Initialize I2C clock	*/
//	__HAL_RCC_I2C1_CLK_ENABLE();

	/*	Initialize SPI clock	*/
	__HAL_RCC_SPI1_CLK_ENABLE();
	__HAL_RCC_SPI2_CLK_ENABLE();

//	/*	Initialize TIM clock	*/
	__HAL_RCC_TIM1_CLK_ENABLE();
	__HAL_RCC_TIM2_CLK_ENABLE();
}














#endif /* Target checking */
