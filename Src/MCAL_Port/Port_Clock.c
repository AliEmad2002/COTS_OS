/*
 * Port_Clock.c
 *
 *  Created on: Sep 25, 2023
 *      Author: Ali Emad
 */


#include "stdint.h"

#include "MCAL_Port/Port_Clock.h"


/*******************************************************************************
 * Private (Static) functions:
 ******************************************************************************/
static void vInitUsbClock(void)
{
	RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
	PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
	vLib_ASSERT(HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) == HAL_OK, 0);
	__HAL_RCC_USB_CLK_ENABLE();
}

/*******************************************************************************
 * API functions:
 ******************************************************************************/
void vPort_Clock_initCpuClock(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

	/** Initializes the RCC Oscillators according to the specified parameters
	* in the RCC_OscInitTypeDef structure.
	*/
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
	RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
	RCC_OscInitStruct.LSEState = RCC_LSE_OFF;
	vLib_ASSERT(HAL_RCC_OscConfig(&RCC_OscInitStruct) == HAL_OK, 0);

	/** Initializes the CPU, AHB and APB buses clocks
	*/
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
								  |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_SYSCLK_DIV1;
	vLib_ASSERT(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) == HAL_OK, 0);
	SystemCoreClockUpdate();
}

void vPort_Clock_initPeriphClock(void)
{
	/*	Initialize GPIO	clock	*/
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_AFIO_CLK_ENABLE();

	/*	Initialize Backup-domain clock	*/
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_RCC_BKP_CLK_ENABLE();

	/*	Initialize ADC clock	*/
	__HAL_RCC_ADC_CONFIG(RCC_ADCPCLK2_DIV6);
	__HAL_RCC_ADC1_CLK_ENABLE();

	/*	Initialize USB clock	*/
	vInitUsbClock();
}












