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
#include "signaling.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

extern uint8_t telem_run;
extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;

extern I2C_HandleTypeDef hi2c1;

//extern IWDG_HandleTypeDef hiwdg;

extern TIM_HandleTypeDef htim2;

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;
extern DMA_HandleTypeDef hdma_usart3_tx;

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
#define HDD_5V_CURR_Pin GPIO_PIN_1
#define HDD_5V_CURR_GPIO_Port GPIOA
#define HDD_12V_CURR_Pin GPIO_PIN_2
#define HDD_12V_CURR_GPIO_Port GPIOA
#define SYS_5V_CURR_Pin GPIO_PIN_3
#define SYS_5V_CURR_GPIO_Port GPIOA
#define SYS_12V_CURR_Pin GPIO_PIN_4
#define SYS_12V_CURR_GPIO_Port GPIOA
#define SYS_5V_V_Pin GPIO_PIN_5
#define SYS_5V_V_GPIO_Port GPIOA
#define SYS_12V_V_Pin GPIO_PIN_6
#define SYS_12V_V_GPIO_Port GPIOA
#define DC_12V_V_Pin GPIO_PIN_7
#define DC_12V_V_GPIO_Port GPIOA
#define DC_5V_V_Pin GPIO_PIN_0
#define DC_5V_V_GPIO_Port GPIOB
#define PS_5V_stb_V_Pin GPIO_PIN_1
#define PS_5V_stb_V_GPIO_Port GPIOB
#define _12V_SEL_Pin GPIO_PIN_2
#define _12V_SEL_GPIO_Port GPIOB
#define RPI_TX_Pin GPIO_PIN_10
#define RPI_TX_GPIO_Port GPIOB
#define RPI_RX_Pin GPIO_PIN_11
#define RPI_RX_GPIO_Port GPIOB
#define PS_PG_Pin GPIO_PIN_12
#define PS_PG_GPIO_Port GPIOB
#define MAIN_SW_Pin GPIO_PIN_13
#define MAIN_SW_GPIO_Port GPIOB
#define RPI_OFF_Pin GPIO_PIN_14
#define RPI_OFF_GPIO_Port GPIOB
#define RPI_FB_Pin GPIO_PIN_15
#define RPI_FB_GPIO_Port GPIOB
#define Buzz_Pin GPIO_PIN_8
#define Buzz_GPIO_Port GPIOA
#define Debug_TX_Pin GPIO_PIN_9
#define Debug_TX_GPIO_Port GPIOA
#define Debug_RX_Pin GPIO_PIN_10
#define Debug_RX_GPIO_Port GPIOA
#define PS_ONOFF_Pin GPIO_PIN_11
#define PS_ONOFF_GPIO_Port GPIOA
#define _5V_SEL_Pin GPIO_PIN_12
#define _5V_SEL_GPIO_Port GPIOA
#define STAT_LED_Pin GPIO_PIN_15
#define STAT_LED_GPIO_Port GPIOA
#define PWR_LED_Pin GPIO_PIN_3
#define PWR_LED_GPIO_Port GPIOB
#define But_ONOFF_Pin GPIO_PIN_4
#define But_ONOFF_GPIO_Port GPIOB
#define SMBALERT_Pin GPIO_PIN_5
#define SMBALERT_GPIO_Port GPIOB
#define TP_Pin GPIO_PIN_8
#define TP_GPIO_Port GPIOB
#define LED_Pin GPIO_PIN_9
#define LED_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

#define LED_ON()		HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET)
#define LED_OFF()		HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET)
#define LED_TOGGLE()	HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin)

#define PWR_LED_ON()        setPwrLed(31)
#define PWR_LED_OFF()       setPwrLed(0)
#define PWR_LED_TOGGLE()    {if(TIM2->CCR2 & 0xFFFF) setPwrLed(0); else setPwrLed(31);}

#define STAT_LED_ON()        setStatLed(31)
#define STAT_LED_OFF()       setStatLed(0)
#define STAT_LED_TOGGLE()    {if(TIM2->CCR1 & 0xFFFF) setStatLed(0); else setStatLed(31);}

#define _5V_SEL_PS()	HAL_GPIO_WritePin(_5V_SEL_GPIO_Port, _5V_SEL_Pin, GPIO_PIN_SET)
#define _5V_SEL_BUF()	HAL_GPIO_WritePin(_5V_SEL_GPIO_Port, _5V_SEL_Pin, GPIO_PIN_RESET)

#define _12V_SEL_PS()	HAL_GPIO_WritePin(_12V_SEL_GPIO_Port, _12V_SEL_Pin, GPIO_PIN_SET)
#define _12V_SEL_BUF()	HAL_GPIO_WritePin(_12V_SEL_GPIO_Port, _12V_SEL_Pin, GPIO_PIN_RESET)

#define Power_SW_READ()     HAL_GPIO_ReadPin(But_ONOFF_GPIO_Port, But_ONOFF_Pin)
#define PS_PG_READ()     HAL_GPIO_ReadPin(PS_PG_GPIO_Port, PS_PG_Pin)
#define RPI_FB_READ()    HAL_GPIO_ReadPin(RPI_FB_GPIO_Port, RPI_FB_Pin)
#define SMBALERT_READ()  HAL_GPIO_ReadPin(SMBALERT_GPIO_Port, SMBALERT_Pin)

#define POWER_ON()		HAL_GPIO_WritePin(MAIN_SW_GPIO_Port, MAIN_SW_Pin, GPIO_PIN_SET)
#define POWER_OFF()		HAL_GPIO_WritePin(MAIN_SW_GPIO_Port, MAIN_SW_Pin, GPIO_PIN_RESET)

#define PS_ON()			HAL_GPIO_WritePin(PS_ONOFF_GPIO_Port, PS_ONOFF_Pin, GPIO_PIN_SET)
#define PS_OFF()		HAL_GPIO_WritePin(PS_ONOFF_GPIO_Port, PS_ONOFF_Pin, GPIO_PIN_RESET)

#define BUZZ_ON()		HAL_GPIO_WritePin(Buzz_GPIO_Port, Buzz_Pin, GPIO_PIN_SET)
#define BUZZ_OFF()		HAL_GPIO_WritePin(Buzz_GPIO_Port, Buzz_Pin, GPIO_PIN_RESET)

#define RPI_POWER_ON()		HAL_GPIO_WritePin(RPI_OFF_GPIO_Port, RPI_OFF_Pin, GPIO_PIN_SET)
#define RPI_POWER_OFF()		HAL_GPIO_WritePin(RPI_OFF_GPIO_Port, RPI_OFF_Pin, GPIO_PIN_RESET)

#define WDR() if(hiwdg.Instance == IWDG) {HAL_IWDG_Refresh(&hiwdg);}

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
