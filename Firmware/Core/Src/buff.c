/*
 * buff.c
 *
 *  Created on: 12.09.2021
 *      Author: bluuu
 */
#include "buff.h"
#include "cli.h"
#include "ltc4015.h"
#include <stdio.h>
#include "main.h"

#define MAX_FILTERS_FL 3
#define FILTSTR_FL  20       // sila filtru

#define MAX_FILTERS_INT 7
#define FILTSTR_INT  8       // sila filtru

#define RX_BUF_SIZE 512

uint8_t  uart_rx_buf[RX_BUF_SIZE];
uint16_t uart_rxtail;
volatile uint32_t buzzer_time;
volatile uint32_t offTim, rpiTout;

uint16_t led_tim_pwr, led_cycles_pwr, led_tim_stat, led_cycles_stat;
static const uint8_t bri_corr[]= {
   0, 1, 2, 3, 4, 5, 7, 9, 12, 15, 18, 22, 27, 32, 38, 44, 51, 58,
   67, 76, 86, 96, 108, 120, 134, 148, 163, 180, 197, 216, 235, 255 };


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

void HAL_SYSTICK_Callback(void)
{
	static uint32_t ledswppwr, ledlevpwr, ledswpstat, ledlevstat;
     if(buzzer_time) { if(--buzzer_time == 0) BUZZ_OFF(); }

     if(led_tim_pwr && ++ledswppwr >= led_tim_pwr)
     {
         ledswppwr = 0;
         if(++ledlevpwr >= 64 + (led_cycles_pwr>>16))
         {
             ledlevpwr = 0;
             led_cycles_pwr--;
             if((led_cycles_pwr & 0xFFFF) == 0) led_tim_pwr = 0;
         }
         if(ledlevpwr>=64) setPwrLed(0); else setPwrLed((ledlevpwr<32) ? ledlevpwr : 63-ledlevpwr);
     } else if(led_tim_pwr == 0) {ledswppwr=0; ledlevpwr=0;}

     if(led_tim_stat && ++ledswpstat >= led_tim_stat)
     {
         ledswpstat = 0;
         if(++ledlevstat >= 64 + (led_cycles_stat>>16))
         {
             ledlevstat = 0;
             led_cycles_stat--;
             if((led_cycles_stat & 0xFFFF) == 0) led_tim_stat = 0;
         }
         if(ledlevstat>=64) setStatLed(0); else setStatLed((ledlevstat<32) ? ledlevstat : 63-ledlevstat);
     } else if(led_tim_stat == 0) {ledswpstat=0; ledlevstat=0;}
}

// UART odbieranie

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    HAL_UART_Receive_IT(&huart1, uart_rx_buf, RX_BUF_SIZE);  // uruchom odbior na przerwaniach
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    HAL_UART_RxCpltCallback(huart);
}

uint16_t UART_has_char()      // zwraca ilosc bajtow w buforze
{
    return (huart1.RxXferSize-huart1.RxXferCount+RX_BUF_SIZE-uart_rxtail) % RX_BUF_SIZE;
}

uint8_t UART_receive()       // odbierz bajt z bufora
{
    uint8_t tmp = uart_rx_buf[uart_rxtail++];
    if(uart_rxtail >= RX_BUF_SIZE) uart_rxtail = 0;
    return tmp;
}

void Buzz(uint32_t tim)   // w ms
{
   buzzer_time = tim;
   BUZZ_ON();
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

void MCUgoSleep()
{
	stop_meas();
    // przygotowanie pinu wakeup
    HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN1);
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
    HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);
    // dobranoc
    printf("Sleep\r\n");
    HAL_PWR_EnterSTANDBYMode();
}


void checkPowerOff()
{
  if(Power_SW_READ() && rpiTout == 0) //nacisniety power, ale jeszcze nie rozpoczeto wylaczania
  {
     Buzz(300);
     if(offTim && HAL_GetTick() - offTim > 2000)    // 2 sekundy naciskania
     {
    	 printf("Wylaczanie w toku ...\r\n");
         rpiTout = HAL_GetTick();  // zaznacz ze wylaczanie w toku i wystartowa³o o...
         RPI_POWER_OFF();           // wymuszenie rpi off
     }
  } else offTim = HAL_GetTick();   // puszczony knefel, uaktualniaj offTim

  if(rpiTout && HAL_GetTick() - rpiTout > 120000)   // rozpoczeto wylaczanie i przekroczono 120 sek na shutdown PI
  {
      rpiTout = 0;
      stop_meas();
      Buzz(1000);
      POWER_OFF();
      printf("Power OFF\r\n");
      HAL_Delay(3000);
      MCUgoSleep();
  }
  if(RPI_FB_READ() == 0 && HAL_GetTick() - rpiTout < 115000)   // rpi potwierdzilo wylaczenie i wiecej niz 5s do konca tout
  {
	  printf("RPI Power off Feedback\r\n");
      rpiTout = HAL_GetTick() - 115000;     // za 5s wylacz system (wczesniejszy if to zrobi, tu tylko przyspieszamy czas)
  }

  if(rpiTout)
  {
	  //... tutaj jakies mruganie (nieblokuj¹ce) sygnalizuj¹ce ¿e trwa wy³¹czanie systemu
  }
}

void CLI() {
    int len = UART_has_char();
    if(len) { for(int i=0; i<len; ++i) CLI_proc(UART_receive()); }
}

void setPwmLed(uint8_t pwm, uint32_t channel)
{
    TIM_OC_InitTypeDef sConfig;
    memset(&sConfig, 0, sizeof(sConfig));
    sConfig.OCMode = (pwm==0) ? TIM_OCMODE_FORCED_INACTIVE : TIM_OCMODE_PWM1;
    sConfig.Pulse = pwm;
    HAL_TIM_OC_ConfigChannel(&htim2, &sConfig, channel);
    HAL_TIM_PWM_Start(&htim2, channel);
}

void setPwrLed(uint8_t bri)
{
    setPwmLed(bri_corr[bri], TIM_CHANNEL_2);
}

void setStatLed(uint8_t bri)
{
    setPwmLed(bri_corr[bri], TIM_CHANNEL_1);
}

void ledSweepPwr(uint16_t spd, uint16_t cnt, uint16_t wait)
{
    led_tim_pwr = spd;
    led_cycles_pwr = cnt | (wait<<16);
}

void ledSweepStat(uint16_t spd, uint16_t cnt, uint16_t wait)
{
    led_tim_stat = spd;
    led_cycles_stat = cnt | (wait<<16);
}
