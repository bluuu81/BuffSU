/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "buff.h"
#include "adc.h"
#include "ltc4015.h"
#include "cli.h"
#include <stdio.h>

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
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;
I2C_HandleTypeDef hi2c1;
IWDG_HandleTypeDef hiwdg;
TIM_HandleTypeDef htim2;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */

//charger limits define
#define RSNB 3
#define RSNI 3

#define CHARGER_VBAT_LO_LIMIT 6.0f
#define CHARGER_VBAT_HI_LIMIT 8.5f
#define CHARGER_VIN_LO_LIMIT 10.8f
#define CHARGER_VIN_HI_LIMIT 12.8f
#define CHARGER_VOLTAGE 8.4f
#define CHARGER_CURRENT 5.5f

uint32_t  ticks, ticks20, ticks100, ticks5s;

float temp_table[3];
float volt_table[7];
int16_t curr_table[6];

uint8_t run_state = 0;
uint8_t stan = 0;
uint8_t ps_pg_state, rpi_feedback, smbalert;
uint8_t ps_count = 0;
uint8_t pg_count = 0;

//charger variables
uint8_t num_cells, chem_type;


enum chem_type
{
	LI_ION_PROG = 0,
	LI_ION_4_2,
	LI_ION_4_1,
	LI_ION_4_0,
	LI_FEPO4_PROG,
	LI_FEPO4_FFC,
	LI_FEPO4_3_6,
	PB_FIX,
	PB_PROG,
};

enum run_state
{
	WARMUP = 0,
	CHECKING,
	INITIALIZING,
	RUNNING_PS,
	RUNNING_BUFFER,
	VOLTAGE_FAIL,
};


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_I2C1_Init(void);
static void MX_IWDG_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART3_UART_Init(void);
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
  MX_USART1_UART_Init();
  printf("Initializing ...\r\n");
  MX_USART3_UART_Init();
  //  MX_IWDG_Init();
  POWER_OFF();
  if (__HAL_PWR_GET_FLAG(PWR_FLAG_SB) != RESET)
  {
    /* Clear Standby flag */
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);
  }
  uint32_t timon = HAL_GetTick();
  while(Power_SW_READ() == GPIO_PIN_SET)
  {
    if(HAL_GetTick() - timon > 1000)     // 1 sekunda naciskania
    {
    	BUZZ_ON();
    	HAL_Delay(1000);
    	BUZZ_OFF();
    	RPI_POWER_ON();
    	timon = HAL_GetTick();
        POWER_ON();    // podtrzymaj zasilanie
    	printf("Power ON\r\n");
    	run_state = CHECKING;
        break;                // przerwij petle while
    }

  //  WDR();    // reset watchdoga
  }
  if(run_state == WARMUP)
  {
      HAL_Delay(300);
      MCUgoSleep();
  }

  MX_DMA_Init();
  HAL_UART_RxCpltCallback(&huart1);
  MX_ADC1_Init();
  MX_I2C1_Init();
  MX_TIM2_Init();
  HAL_TIM_Base_Init(&htim2);
  HAL_TIM_Base_Start(&htim2);
  HAL_TIM_PWM_Init(&htim2);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
  /* USER CODE BEGIN 2 */

  ADC_DMA_Start();
  HAL_Delay(10);


  if (LTC4015_check())
  {
	num_cells = LTC4015_get_cells();
	chem_type = LTC4015_get_chem();
	printf("Chem type : %d   Num cells : %d \r\n", chem_type, num_cells);
	LTC4015_init(); //pusta
	LTC4015_VBAT_limits(CHARGER_VBAT_LO_LIMIT, CHARGER_VBAT_HI_LIMIT, num_cells, chem_type);
	HAL_Delay(10);
	LTC4015_VIN_limits(CHARGER_VIN_LO_LIMIT,CHARGER_VIN_HI_LIMIT);
	HAL_Delay(10);
	LTC4015_set_charge_voltage(CHARGER_VOLTAGE, num_cells, chem_type);
	HAL_Delay(10);
	LTC4015_set_charge_current(CHARGER_CURRENT, RSNB);
	stop_run_bsr();
	HAL_Delay(100);
	disable_jeita();
	HAL_Delay(1000);
	start_charging();
	HAL_Delay(1000);
  }


  ticks = HAL_GetTick();
  ticks20 = HAL_GetTick();
  ticks100 = HAL_GetTick();
  ticks5s = HAL_GetTick();
//  STAT_LED_ON();
//  PWR_LED_ON();
  while (1)
  {
    /* USER CODE END WHILE */
	  checkPowerOff();
	  CLI();
	  if(debug_level > 2) debug_level = 0;
	  if(HAL_GetTick()-ticks20 >= 20)
	  {
	      ticks20 = HAL_GetTick();
	      ps_pg_state = PS_PG_READ();
	      rpi_feedback = RPI_FB_READ();
	      smbalert = SMBALERT_READ();
	      supply_check_select();
	  }

	  if(HAL_GetTick()-ticks100 >= 100)
	  {
	      ticks100 = HAL_GetTick();
	      fill_temperature_table();
	      fill_voltage_table();
	      fill_current_table();
	  }

	  if(HAL_GetTick()-ticks >= 1000)
	  {
	      ticks = HAL_GetTick();
//	      PWR_LED_TOGGLE();
	      switch (debug_level)
	      {
	      	  case 1:
	      		  print_regs();
	      		  break;
	      	  case 2:
	      		  print_volt_curr();
	      		  break;

	      	  case 0:
	      	  default:
	      		  break;
	      }

	   }

	  if(HAL_GetTick()-ticks5s >= 5000)
	  {
		  ticks5s = HAL_GetTick();
//		  STAT_LED_TOGGLE();
//		  print_regs();
//		  start_charging();
	  }

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

void print_volt_curr()
{
//	printf("Temp round: %3.1f\r\n", temp_table[0]);
//	printf("5V SYS: %2.2f  ", volt_table[0]);
//	printf("5V DC: %2.2f \r\n", volt_table[1]);
//	printf("12V SYS: %2.2f \r\n", volt_table[3]);
//	printf("5V STB: %2.2f \r\n", volt_table[2]);
	printf("Vin: %2.2f  V , ", volt_table[5]);
	printf("Vbat: %2.2f V\r\n", volt_table[6]);
	printf("5V SYS Curr : %d mA , ", curr_table[0]);
	printf("5V HDD Curr : %d mA\r\n", curr_table[2]);
	printf("12V SYS Curr : %d mA , ", curr_table[1]);
	printf("12V HDD Curr : %d mA\r\n", curr_table[3]);
	printf("IIN Curr : %d mA , ", curr_table[4]);
	printf("IBAT Curr : %d mA\r\n", curr_table[5]);
	printf("SMBALERT : %d \r\n", smbalert);
	printf("DIE TEMP : %2.2f deg C\r\n", temp_table[1]);
}


void print_regs()
{
//	  read_register_val(0x0D);
//	  read_register_val(0x0E);
//	  read_register_val(0x0F);
//	  read_register_val(0x14);
//	  read_register_val(0x29);
	  printf("--- CHARGER STATE ---\r\n");
	  read_register_val(0x34);
	  printf("--- CHARGE STATUS ---\r\n");
	  read_register_val(0x35);
	  printf("--- LIMIT ALERTS ---\r\n");
	  read_register_val(0x36);
	  printf("--- CHARGER STATE ALERTS ---\r\n");
	  read_register_val(0x37);
	  printf("--- CHARGE STATUS ALERTS ---\r\n");
	  read_register_val(0x38);
	  printf("--- SYSTEM STATUS ---\r\n");
	  read_register_val(0x39);
//	  read_register_val(0x45);
//	  read_register_val(0x47);
//	  read_register_val(0x4A);
}

void fill_temperature_table(void)
{
    temp_table[0] = Round_filter_fl(1,GET_Buff_Temp());
    temp_table[1] = Round_filter_fl(2,LTC4015_get_dietemp());
    temp_table[2] = 0; // TODO: NTC Temp
}

void fill_voltage_table(void)
{
    volt_table[0] = GET_SYS_5V();
    volt_table[1] = GET_DC_5V();
    volt_table[2] = GET_STB_5V();
    volt_table[3] = GET_SYS_12V();
    volt_table[4] = GET_DC_12V();
    volt_table[5] = LTC4015_get_vin();
    volt_table[6] = LTC4015_get_vbat(num_cells, chem_type);
}

void fill_current_table(void)
{
    curr_table[0] = Round_filter_int(1,GET_SYS_5V_CURR());
    curr_table[1] = Round_filter_int(2,GET_SYS_12V_CURR());
    curr_table[2] = Round_filter_int(3,GET_HDD_5V_CURR());
    curr_table[3] = Round_filter_int(4,GET_HDD_12V_CURR());
    curr_table[4] = Round_filter_int(5,LTC4015_get_iin(RSNI));
	curr_table[5] = Round_filter_int(6,LTC4015_get_ibat(RSNB));
}

void supply_check_select()
{
	if((volt_table[2] <= 5.4f && volt_table[2] >= 4.5f) && ps_pg_state == 0 && (run_state == RUNNING_BUFFER || run_state == CHECKING)) ps_count++;
	else ps_count = 0;
	if(ps_count > 8 && ps_count < 12) { PS_ON(); printf("PS-ON ! \r\n"); }
	if (ps_pg_state == 1) pg_count++;
	else pg_count = 0;
	if(pg_count > 10 && run_state == RUNNING_BUFFER)
	{
		ps_count = 0;
		pg_count = 0;
		_5V_SEL_PS();
		_12V_SEL_PS();
		run_state = RUNNING_PS;
		printf("PS select \r\n");
//		PWR_LED_ON();
	}


	if (ps_pg_state == 0 && (run_state == RUNNING_PS || run_state == CHECKING))
	{
		_5V_SEL_BUF();
		_12V_SEL_BUF();
		run_state = RUNNING_BUFFER;
		printf("Buffer select \r\n");
//		ledSweepPwr(4,0xFFFF,30);
	}
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */
  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 10;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  sConfig.SamplingTime = ADC_SAMPLETIME_28CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_2;
  sConfig.Rank = ADC_REGULAR_RANK_3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_3;
  sConfig.Rank = ADC_REGULAR_RANK_4;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = ADC_REGULAR_RANK_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = ADC_REGULAR_RANK_6;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_6;
  sConfig.Rank = ADC_REGULAR_RANK_7;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_7;
  sConfig.Rank = ADC_REGULAR_RANK_8;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_8;
  sConfig.Rank = ADC_REGULAR_RANK_9;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_9;
  sConfig.Rank = ADC_REGULAR_RANK_10;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_16_9;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief IWDG Initialization Function
  * @param None
  * @retval None
  */
static void MX_IWDG_Init(void)
{

  /* USER CODE BEGIN IWDG_Init 0 */

  /* USER CODE END IWDG_Init 0 */

  /* USER CODE BEGIN IWDG_Init 1 */

  /* USER CODE END IWDG_Init 1 */
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_4;
  hiwdg.Init.Reload = 4095;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN IWDG_Init 2 */

  /* USER CODE END IWDG_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 562;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 256;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 50;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_LOW;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 12, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, _12V_SEL_Pin|MAIN_SW_Pin|RPI_OFF_Pin|PWR_LED_Pin
                          |LED1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, TP2_Pin|PS_ONOFF_Pin|_5V_SEL_Pin|STAT_LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : PC13 PC14 PC15 */
  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : _12V_SEL_Pin MAIN_SW_Pin RPI_OFF_Pin PWR_LED_Pin
                           LED1_Pin */
  GPIO_InitStruct.Pin = _12V_SEL_Pin|MAIN_SW_Pin|RPI_OFF_Pin|PWR_LED_Pin
                          |LED1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PS_PG_Pin RPI_FB_Pin But_ONOFF_Pin SMBALERT_Pin */
  GPIO_InitStruct.Pin = PS_PG_Pin|RPI_FB_Pin|But_ONOFF_Pin|SMBALERT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : TP2_Pin PS_ONOFF_Pin _5V_SEL_Pin STAT_LED_Pin */
  GPIO_InitStruct.Pin = TP2_Pin|PS_ONOFF_Pin|_5V_SEL_Pin|STAT_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

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
  __disable_irq();
  while (1)
  {
  }
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
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
