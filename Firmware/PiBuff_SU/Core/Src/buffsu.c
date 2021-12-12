/*
 * buffsu.c
 *
 *  Created on: Dec 11, 2021
 *      Author: bluuu
 */

#include "main.h"
#include "buffsu.h"
#include "pwm.h"
#include "cli.h"
#include <stdio.h>

// LTC4015 settings
#define RSNB 3
#define RSNI 3

#define CHARGER_VBAT_LO_LIMIT 6.0f
#define CHARGER_VBAT_HI_LIMIT 8.5f
#define CHARGER_VIN_LO_LIMIT 10.8f
#define CHARGER_VIN_HI_LIMIT 12.8f
#define CHARGER_VOLTAGE 8.4f
#define CHARGER_CURRENT 5.5f

uint32_t ticks1s; //, ticks20ms, ticks100ms, ticks5s;
volatile uint32_t offTim, rpiTout, beepTim;

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



//functions

void BuffSU_Loop()
{
	PWM_Init_Timers();
	ticks1s = HAL_GetTick();
	ledSweepStat(4,0xFFFF,30);

	while(1)
	{
		checkPowerOff();

		if(HAL_GetTick()-ticks1s >= 1000)
		{
			ticks1s = HAL_GetTick();
//			PWR_LED_TOGGLE();
		}
		WDR();
	}
}

// Startup checking

void startup_check()
{
	  POWER_OFF();
	  if (__HAL_PWR_GET_FLAG(PWR_FLAG_SB) != RESET)
	  {
	    /* Clear Standby flag */
	    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);
	  }
	  uint32_t timon = HAL_GetTick();
	  while(Power_SW_READ() == GPIO_PIN_SET)
	  {
	    if(HAL_GetTick() - timon > 1000)     // 1 sec push
	    {
	    	Buzz(1000);
	    	HAL_Delay(500);
	    	RPI_POWER_ON();
	    	timon = HAL_GetTick();
	        POWER_ON();    // pullup power supply
	    	printf("Power ON\r\n");
	    	run_state = CHECKING;
	    	WDR();
	        break;                // break while loop
	    }

	    WDR();    // watchdog reset
	  }
	  if(run_state == WARMUP)
	  {
	      HAL_Delay(300);
	      MCUgoSleep();
	  }
}

void MCUgoSleep()
{
//	stop_meas();
    // prepare wake-up pin
    HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN1);
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
    HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);
    // go sleep
    printf("Sleep\r\n");
    HAL_PWR_EnterSTANDBYMode();
}

void checkPowerOff()
{
  if(Power_SW_READ() && rpiTout == 0) // power button pressed
  {
     Buzz(1000);
     if(offTim && HAL_GetTick() - offTim > 2000)    // 2 sekundy naciskania
     {
    	 PWR_LED_OFF();
    	 STAT_LED_OFF();
    	 ledSweepStat(10,10,5);
    	 ledSweepPwr(10,10,5);
    	 printf("Power off in progress ...\r\n");
         rpiTout = HAL_GetTick();  // mark power off proccess time
         RPI_POWER_OFF();           // power off RPi by PIN command
     }
  } else offTim = HAL_GetTick();   // power button released

  if(rpiTout && HAL_GetTick() - rpiTout > 120000)   // RPi power down in progress and timeout 120s
  {
      rpiTout = 0;
//      stop_meas();
      Buzz(1000);
 	  ledSweepStat(3,10,5);
      POWER_OFF();
      printf("Power OFF\r\n");
      HAL_Delay(3000);
      MCUgoSleep();
  }
  if(RPI_FB_READ() == 0 && HAL_GetTick() - rpiTout < 115000)   // RPi power off confirmed and 5s to end
  {
	  printf("RPI Power off Feedback\r\n");
      rpiTout = HAL_GetTick() - 115000;     // 5s to system off
  }

  if(rpiTout)
  {
 	  ledSweepPwr(3,0xFFFF,15);
	  ledSweepStat(3,0xFFFF,30);
		if(HAL_GetTick()-beepTim >= 1500)
		{
			beepTim = HAL_GetTick();
		 	Buzz(300);
		}
  }
}
