/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
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

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "cordic.h"
#include "dac.h"
#include "dma.h"
#include "usart.h"
#include "opamp.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
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

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
__unused int _write(__unused int file, char *ptr, int len) {
  HAL_UART_Transmit(&hlpuart1, (uint8_t *)ptr, len, len);
}
#define ADC_BUF_LEN 5000
uint16_t adcBuffer[ADC_BUF_LEN];

volatile bool adcFinished = 1;
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
  adcFinished = true;
}


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
  MX_LPUART1_UART_Init();
  MX_CORDIC_Init();
  MX_OPAMP2_Init();
  MX_TIM8_Init();
  MX_DMA_Init();
  MX_ADC2_Init();
  MX_DAC1_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start(&htim8);
  HAL_TIM_OC_Start(&htim8, TIM_CHANNEL_1);
  HAL_TIMEx_OCN_Start(&htim8, TIM_CHANNEL_1);
  HAL_TIM_OC_Start(&htim8, TIM_CHANNEL_2);
  HAL_TIMEx_OCN_Start(&htim8, TIM_CHANNEL_2);
  HAL_DAC_Start(&hdac1,DAC_CHANNEL_1);
  HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_1,DAC_ALIGN_12B_R,2048);
  HAL_OPAMP_Start(&hopamp2);
  HAL_ADCEx_Calibration_Start(&hadc2, ADC_DIFFERENTIAL_ENDED);
  while (HAL_ADC_GetState(&hadc2) == HAL_ADC_STATE_REG_BUSY) {
  }
  /* USER CODE END 2 */
 
 

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
  while (1) {
    long long maxDist = 0;
    int maxDistIdx = 0;
    long long minDist = ULONG_LONG_MAX;
    int minDistIdx = 0;


    long long maxDistDiff = 0;
    int maxDistIdxDiff = 0;
    long long minDistDiff = ULONG_LONG_MAX;
    int minDistIdxDiff = 0;

    unsigned long dists[2000];

    for (int shift = 0; shift <2000; shift+= 1) {
      /* USER CODE END WHILE */

      /* USER CODE BEGIN 3 */

      //    HAL_Delay(100);
      HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
      //    status = HAL_ADC_Stop_DMA(&hadc2);
      __HAL_TIM_SET_COUNTER(&htim8,0);
      __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_1, shift);
      __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_2, 1999 - shift);

      HAL_Delay(2); // let acoustics to be stabilized
      adcFinished = false;
      HAL_SuspendTick();
      HAL_StatusTypeDef status =
          HAL_ADC_Start_DMA(&hadc2, (uint32_t *)&adcBuffer, ADC_BUF_LEN);
      while (!adcFinished) {
        __WFI();
      }
      HAL_ADC_Stop_DMA(&hadc2);
      HAL_ResumeTick();

      unsigned long sum = 0;
      unsigned long long distort = 0;
      for(int i = 0; i < ADC_BUF_LEN; ++i) {
        sum +=  adcBuffer[i];
        distort +=  adcBuffer[i] * adcBuffer[i];
      }
      distort -= (unsigned long long)sum * sum / ADC_BUF_LEN;
//      printf("S: %d; D: %lld\n", shift, distort / ADC_BUF_LEN);
      /*## Configure the CORDIC peripheral
       * ####################################*/
      dists[shift] = distort / ADC_BUF_LEN;
    }
    __NOP();
  }
#pragma clang diagnostic pop
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Configure the main internal regulator output voltage 
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV6;
  RCC_OscInitStruct.PLL.PLLN = 80;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the peripherals clocks 
  */
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_LPUART1|RCC_PERIPHCLK_ADC12;
  PeriphClkInit.Lpuart1ClockSelection = RCC_LPUART1CLKSOURCE_PCLK1;
  PeriphClkInit.Adc12ClockSelection = RCC_ADC12CLKSOURCE_SYSCLK;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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
  (void) file;
  (void) line;
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
