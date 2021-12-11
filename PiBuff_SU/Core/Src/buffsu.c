/*
 * buffsu.c
 *
 *  Created on: Dec 11, 2021
 *      Author: bluuu
 */

#include "main.h"
#include "buffsu.h"
#include "pwm.h"

uint32_t ticks1s; //, ticks20ms, ticks100ms, ticks5s;

void BuffSU_Loop()
{
	POWER_ON();

	PWM_Init_Timers();
	ticks1s = HAL_GetTick();
	PWR_LED_OFF();
	STAT_LED_ON();
	Buzz(500);
	ledSweepStat(4,0xFFFF,30);

	while(1)
	{


//	  if(HAL_GetTick()-ticks1s >= 1000)
//	  {
//	  	  ticks1s = HAL_GetTick();
//		  PWR_LED_TOGGLE();
//		  STAT_LED_TOGGLE();
//	  }
//	  WDR();
	}
}
