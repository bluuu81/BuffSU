/*
 * signaling.c
 *
 *  Created on: 18.12.2021
 *      Author: bluuu
 */

#include "signaling.h"
#include "main.h"

volatile uint32_t buzzer_time;

uint16_t led_tim_pwr, led_cycles_pwr, led_tim_stat, led_cycles_stat;

static const uint8_t bri_corr[]= {
   0, 1, 2, 3, 4, 5, 7, 9, 12, 15, 18, 22, 27, 32, 38, 44, 51, 58,
   67, 76, 86, 96, 108, 120, 134, 148, 163, 180, 197, 216, 235, 255 };

void setPwrLed(uint8_t bri)
{
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, bri_corr[bri]);
}

void setStatLed(uint8_t bri)
{
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, bri_corr[bri]);
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

void PWM_Init_Timers()
{
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
}

void Buzz(uint32_t tim)   // ms
{
   buzzer_time = tim;
   BUZZ_ON();
}

void HAL_SYSTICK_Callback()
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

