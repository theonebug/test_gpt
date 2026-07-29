/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "bsp_led.h"
#include "bsp_button.h"
#include "bsp_encoder.h"
#include "input.h"
#include "event.h"
#include "bsp_lcd.h"
#include "st7789.h"
#include "ok_32.h"
#include "device.h"
#include "sync.h"
#include "tiristor.h"
#include "settings.h"
#include "menu.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
//static uint16_t Angle = 90;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SPI2_Init();
  MX_TIM2_Init();
  MX_TIM4_Init();
  MX_USART2_UART_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  //HAL_TIM_Base_Start_IT(&htim3);
  //HAL_TIM_Base_Start(&htim1);
  SYNC_Init();
  BSP_LED_Init();
  EVENT_Init();
  INPUT_Init();
  Device_Init();
  BSP_LCD_UpdateAngle(Device_GetAngle());
  Tiristor_Init();
  SETTINGS_Init();
  MENU_Init();

  // 1. Аппаратно включаем сам счетчик таймера TIM2 (устанавливаем бит CR1_CEN)
  __HAL_TIM_ENABLE(&htim2);

  // 2. Разрешаем прерывание таймера в контроллере прерываний STM32 (NVIC)
  HAL_NVIC_EnableIRQ(TIM2_IRQn);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  BSP_LCD_Init();



  BSP_LCD_SetScreen(LCD_SCREEN_MAIN);
  BSP_LCD_UpdateAngle(Device.Angle);
  BSP_LCD_UpdateMode(0);
  BSP_LCD_UpdateRS485(1);
  BSP_LCD_UpdateSync(1);

  BSP_LCD_UpdateStatus("READY", UI_COLOR_OK);
  BSP_LCD_UpdateDuration(250);
  BSP_LCD_UpdateProgress(75);

  EventMessage_t msg;
  while (1)
  {
      INPUT_Update();
      SYNC_Update();
      Device_Update();
      SETTINGS_Process();

      //BSP_LCD_UpdateDuration((uint16_t)Tiristor_GetTestCounter());
      //BSP_LCD_UpdateDuration((htim1.Instance->CR1 & TIM_CR1_CEN) ? 1 : 0);
      //BSP_LCD_UpdateDuration(Tiristor_GetDelayIrqCounter());
      //BSP_LCD_UpdateDuration(SYNC_GetFrequency_x10());

      static uint32_t SyncTimer = 0;

          if((HAL_GetTick() - SyncTimer >= 200) && !MENU_IsActive())
          {
              SyncTimer = HAL_GetTick();

              BSP_LCD_UpdateSync(SYNC_IsPresent());

              BSP_LCD_UpdateAngle(Device_GetAngle());
          }

          static uint32_t PrevSync    = 0;
          static uint32_t PrevGlitch  = 0;
          static uint32_t PrevCH1     = 0;
          static uint32_t PrevRestart = 0;
          static uint32_t Tick = 0;

          if((HAL_GetTick() - Tick >= 1000) && !MENU_IsActive())
          {
              Tick = HAL_GetTick();

              uint32_t sync    = SYNC_GetCounter();
              uint32_t glitch  = SYNC_GetGlitchCounter();
              uint32_t ch1     = Tiristor_GetCH1Counter();
              uint32_t restart = Tiristor_GetRestartCounter();

              BSP_LCD_UpdateDebug((uint16_t)(sync    - PrevSync),
                                  (uint16_t)(glitch  - PrevGlitch),
                                  (uint16_t)(ch1     - PrevCH1),
                                  (uint16_t)(restart - PrevRestart));

              PrevSync    = sync;
              PrevGlitch  = glitch;
              PrevCH1     = ch1;
              PrevRestart = restart;
          }

      while(EVENT_Get(&msg))
      {
          /* Когда открыто меню настроек, энкодер работает на меню */
          if(MENU_HandleEvent(&msg))
          {
              continue;
          }

          /* Когда устройство управляет тиристором, доступна только STOP */
          if(!Device_IsIdle() && (msg.Id != EVENT_STOP_CLICK))
          {
              continue;
          }

          switch(msg.Id)
          {
              case EVENT_RUN_CLICK:
                  Device_Start();
                  break;

              case EVENT_STOP_CLICK:
                  Device_Stop();
                  break;

              case EVENT_ENCODER_LONG:
                  /* Длительное нажатие кнопки энкодера - меню настроек */
                  MENU_Open();
                  break;

              case EVENT_ENCODER_LEFT:
                  Device_SetAngle(Device_GetAngle() + 1);
                  break;

              case EVENT_ENCODER_RIGHT:
                  Device_SetAngle(Device_GetAngle() - 1);
                  break;

              default:
                  break;
          }
      }
      BSP_LCD_Update();
  }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */


  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
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
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if(GPIO_Pin == SYNC_IN_Pin)
    {
        SYNC_EXTI_Handler();
    }
}

//void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
//{
//    if(htim->Instance == TIM2)
//    {
//        if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
//        {
//            //TestCounter = 1111;
//            Tiristor_Channel1_IRQHandler();
//        }
//        else if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
//        {
//            //TestCounter = 2222;
//            Tiristor_Channel2_IRQHandler();
//        }
//    }
//}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
