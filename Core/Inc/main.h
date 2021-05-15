/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define WDI_Pin GPIO_PIN_8
#define WDI_GPIO_Port GPIOF
#define FEC_nRST_Pin GPIO_PIN_10
#define FEC_nRST_GPIO_Port GPIOF
#define conn_5R_Pin GPIO_PIN_14
#define conn_5R_GPIO_Port GPIOB

#define conn_10R_Pin GPIO_PIN_7
#define conn_10R_GPIO_Port GPIOG
#define conn_2400R_Pin GPIO_PIN_6
#define conn_2400R_GPIO_Port GPIOG

#define FLOAT_TEST_Pin GPIO_PIN_2
#define FLOAT_TEST_GPIO_Port GPIOG
#define RS_TEST_Pin GPIO_PIN_3
#define RS_TEST_GPIO_Port GPIOG
#define GAIN_2_Pin GPIO_PIN_7
#define GAIN_2_GPIO_Port GPIOC
#define GAIN_1_Pin GPIO_PIN_8
#define GAIN_1_GPIO_Port GPIOC
#define GAIN_0_Pin GPIO_PIN_9
#define GAIN_0_GPIO_Port GPIOC
#define TESTLED_R_Pin GPIO_PIN_2
#define TESTLED_R_GPIO_Port GPIOD
#define TESTLED_G_Pin GPIO_PIN_3
#define TESTLED_G_GPIO_Port GPIOD
#define RSLED_Pin GPIO_PIN_4
#define RSLED_GPIO_Port GPIOD
#define WRKLED_R_Pin GPIO_PIN_5
#define WRKLED_R_GPIO_Port GPIOD
#define WRKLED_G_Pin GPIO_PIN_6
#define WRKLED_G_GPIO_Port GPIOD
#define SPI_nCS_Pin GPIO_PIN_4
#define SPI_nCS_GPIO_Port GPIOB
#define EN_COILPWR_Pin GPIO_PIN_8
#define EN_COILPWR_GPIO_Port GPIOB
#define PWM_Pin GPIO_PIN_8
#define PWM_GPIO_Port GPIOA

/* #define SDRAM_MEMORY_WIDTH            FMC_SDRAM_MEM_BUS_WIDTH_8  */
/* #define SDRAM_MEMORY_WIDTH            FMC_SDRAM_MEM_BUS_WIDTH_16 */
#define SDRAM_MEMORY_WIDTH               FMC_SDRAM_MEM_BUS_WIDTH_32

#define SDCLOCK_PERIOD                   FMC_SDRAM_CLOCK_PERIOD_2
/* #define SDCLOCK_PERIOD                FMC_SDRAM_CLOCK_PERIOD_3 */

#define SDRAM_TIMEOUT     ((uint32_t)0xFFFF)

#define SDRAM_MODEREG_BURST_LENGTH_1             ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2             ((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4             ((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8             ((uint16_t)0x0004)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     ((uint16_t)0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_2              ((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3              ((uint16_t)0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     ((uint16_t)0x0200)

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
