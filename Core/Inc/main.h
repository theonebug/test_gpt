/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
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
/* Обновление сторожевого таймера внутри длительных операций (отрисовка экрана) */
void Watchdog_Refresh(void);

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define TIRISTOR_OUT_Pin GPIO_PIN_14
#define TIRISTOR_OUT_GPIO_Port GPIOC
#define SYNC_IN_Pin GPIO_PIN_0
#define SYNC_IN_GPIO_Port GPIOA
#define SYNC_IN_EXTI_IRQn EXTI0_IRQn
#define RS485_TX_Pin GPIO_PIN_2
#define RS485_TX_GPIO_Port GPIOA
#define RS485_RX_Pin GPIO_PIN_3
#define RS485_RX_GPIO_Port GPIOA
#define RS485_DE_Pin GPIO_PIN_5
#define RS485_DE_GPIO_Port GPIOA
#define BTN_RUN_Pin GPIO_PIN_7
#define BTN_RUN_GPIO_Port GPIOA
#define BTN_STOP_Pin GPIO_PIN_0
#define BTN_STOP_GPIO_Port GPIOB
#define LED_READY_Pin GPIO_PIN_1
#define LED_READY_GPIO_Port GPIOB
#define LED_ALARM_Pin GPIO_PIN_10
#define LED_ALARM_GPIO_Port GPIOB
#define LED_PULSE_Pin GPIO_PIN_11
#define LED_PULSE_GPIO_Port GPIOB
#define LCD_DC_Pin GPIO_PIN_12
#define LCD_DC_GPIO_Port GPIOB
#define LCD_SCK_Pin GPIO_PIN_13
#define LCD_SCK_GPIO_Port GPIOB
#define LCD_CS_Pin GPIO_PIN_14
#define LCD_CS_GPIO_Port GPIOB
#define LCD_SDA_Pin GPIO_PIN_15
#define LCD_SDA_GPIO_Port GPIOB
#define SYNC_OSC_Pin GPIO_PIN_8
#define SYNC_OSC_GPIO_Port GPIOA
#define LCD_RST_Pin GPIO_PIN_15
#define LCD_RST_GPIO_Port GPIOA
#define ENC_A_Pin GPIO_PIN_6
#define ENC_A_GPIO_Port GPIOB
#define ENC_B_Pin GPIO_PIN_7
#define ENC_B_GPIO_Port GPIOB
#define ENC_BTN_Pin GPIO_PIN_8
#define ENC_BTN_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
