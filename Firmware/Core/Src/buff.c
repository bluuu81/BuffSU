/*
 * buff.c
 *
 *  Created on: 12.09.2021
 *      Author: bluuu
 */
#include "buff.h"
#include <stdio.h>

#define MAX_FILTERS_FL 1
#define FILTSTR_FL  20       // sila filtru

#define MAX_FILTERS_INT 4
#define FILTSTR_INT  8       // sila filtru



  // printf redirection to UART1
  int _write(int file, char *ptr, int len)
  {
      HAL_UART_Transmit(&huart1, (uint8_t *)ptr, len, len+1);  // debug uart
      return len;
  }

  void debug_putchar(uint8_t ch)
  {
      HAL_UART_Transmit(&huart1, &ch, 1, 2);  // debug uart
  }


float Round_filter_fl(uint8_t index, float value)      // index filtru
{
   static float filter[MAX_FILTERS_FL];
   if(index > MAX_FILTERS_FL-1) index = MAX_FILTERS_FL-1;  // zabezpieczenie
   if(filter[index] == 0) filter[index] = value * FILTSTR_FL;   // inicjalizacja pustego filtru
   filter[index] -= filter[index] / FILTSTR_FL;
   filter[index] += value;
   return (filter[index] / FILTSTR_FL);
}

int16_t Round_filter_int(uint8_t index, int16_t value)      // index filtru
{
   static float filter[MAX_FILTERS_INT];
   if(index > MAX_FILTERS_INT-1) index = MAX_FILTERS_INT-1;  // zabezpieczenie
   if(filter[index] == 0) filter[index] = value * FILTSTR_INT;   // inicjalizacja pustego filtru
   filter[index] -= filter[index] / FILTSTR_INT;
   filter[index] += value;
   return (filter[index] / FILTSTR_INT);
}

void checkPowerOff()
{
  static uint32_t checktim = 0;
  static uint32_t presscnt = 0;
  uint32_t currtim = HAL_GetTick();
  if(currtim-checktim >= 100)          // wykonuj co 100ms
  {
     checktim = currtim;
     if(Power_SW_READ() == GPIO_PIN_SET)  presscnt++; else presscnt = 0;
     if(presscnt > 20)          // 2 sekundy naciskania
     {
         POWER_OFF();
         printf("Power OFF\r\n");
         // przygotowanie pinu wakeup
         HAL_Delay(3000);
         HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN1);
         __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
         HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);
         // dobranoc
         printf("Sleep\r\n");
         HAL_PWR_EnterSTANDBYMode();
     }
  }
}

