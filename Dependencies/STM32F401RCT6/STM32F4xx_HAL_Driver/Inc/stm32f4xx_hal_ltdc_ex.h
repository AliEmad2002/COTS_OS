/**
  ******************************************************************************
  * @file    stm32f4xx_hal_ltdc_ex.h
  * @author  MCD Application Team
  * @brief   Header file of LTDC HAL Extension module.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2016 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/*	Target checking	*/
#include "MCAL_Port/Port_Target.h"
#ifdef MCAL_PORT_TARGET_STM32F401RCT6


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STM32F4xx_HAL_LTDC_EX_H
#define STM32F4xx_HAL_LTDC_EX_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal_def.h"

#if defined (LTDC) && defined (DSI)

#include "stm32f4xx_hal_dsi.h"

/** @addtogroup STM32F4xx_HAL_Driver
  * @{
  */

/** @addtogroup LTDCEx
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
/** @addtogroup LTDCEx_Exported_Functions
  * @{
  */

/** @addtogroup LTDCEx_Exported_Functions_Group1
  * @{
  */
HAL_StatusTypeDef HAL_LTDCEx_StructInitFromVideoConfig(LTDC_HandleTypeDef *hltdc, DSI_VidCfgTypeDef *VidCfg);
HAL_StatusTypeDef HAL_LTDCEx_StructInitFromAdaptedCommandConfig(LTDC_HandleTypeDef *hltdc, DSI_CmdCfgTypeDef *CmdCfg);
/**
  * @}
  */

/**
  * @}
  */

/* Private types -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @}
  */

/**
  * @}
  */

#endif /* LTDC && DSI */

#ifdef __cplusplus
}
#endif

#endif /* STM32F4xx_HAL_LTDC_EX_H */


#endif /* Target checking */