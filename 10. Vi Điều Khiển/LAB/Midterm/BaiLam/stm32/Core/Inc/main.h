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
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
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
#include "stm32f1xx_hal.h"

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

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define Led_Test_MCU_And_Timer_Pin GPIO_PIN_0
#define Led_Test_MCU_And_Timer_GPIO_Port GPIOA
#define Led_1_Pin GPIO_PIN_1
#define Led_1_GPIO_Port GPIOA
#define Led_2_Pin GPIO_PIN_2
#define Led_2_GPIO_Port GPIOA
#define Led_3_Pin GPIO_PIN_3
#define Led_3_GPIO_Port GPIOA
#define Led_4_Pin GPIO_PIN_7
#define Led_4_GPIO_Port GPIOA
#define Btn_1_Pin GPIO_PIN_0
#define Btn_1_GPIO_Port GPIOB
#define Btn_2_Pin GPIO_PIN_1
#define Btn_2_GPIO_Port GPIOB
#define Btn_3_Pin GPIO_PIN_2
#define Btn_3_GPIO_Port GPIOB
#define Fsm_Led_2_Pin GPIO_PIN_10
#define Fsm_Led_2_GPIO_Port GPIOB
#define Led_5_Pin GPIO_PIN_8
#define Led_5_GPIO_Port GPIOA
#define Led_6_Pin GPIO_PIN_9
#define Led_6_GPIO_Port GPIOA
#define Led_7_Pin GPIO_PIN_10
#define Led_7_GPIO_Port GPIOA
#define Led_8_Pin GPIO_PIN_11
#define Led_8_GPIO_Port GPIOA
#define Led_9_Pin GPIO_PIN_12
#define Led_9_GPIO_Port GPIOA
#define Led_10_Pin GPIO_PIN_13
#define Led_10_GPIO_Port GPIOA
#define Led_11_Pin GPIO_PIN_14
#define Led_11_GPIO_Port GPIOA
#define Led_12_Pin GPIO_PIN_15
#define Led_12_GPIO_Port GPIOA
#define Fsm_Led_0_Pin GPIO_PIN_8
#define Fsm_Led_0_GPIO_Port GPIOB
#define Fsm_Led_1_Pin GPIO_PIN_9
#define Fsm_Led_1_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
