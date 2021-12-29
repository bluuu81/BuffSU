/*
 * buff.c
 *
 *  Created on: 19.12.2021
 *      Author: bluuu
 */

#include "buff.h"
#include "main.h"
#include "ltc4015.h"
#include "adc.h"
#include "config.h"
#include <string.h>
#include <stdio.h>

#define MAX_FILTERS_FL 3
#define FILTSTR_FL  20       // filter strenght

#define MAX_FILTERS_INT 7
#define FILTSTR_INT  8       // filter strenght

volatile uint8_t run_state = 0;
volatile uint8_t ps_count = 0;
volatile uint8_t pg_count = 0;

volatile uint32_t offTim, rpiTout, beepTim;

enum run_state
{
	WARMUP = 0,
	CHECKING,
	INITIALIZING,
	RUNNING_PS,
	RUNNING_BUFFER,
	VOLTAGE_FAIL,
};

void BUFF_fill_values()
{
	fill_temperature_table();
	fill_voltage_table();
	fill_current_table();
}

float Round_filter_fl(uint8_t index, float value)      // filtr index
{
   static float filter[MAX_FILTERS_FL];
   if(index > MAX_FILTERS_FL-1) index = MAX_FILTERS_FL-1;  // protection
   if(filter[index] == 0) filter[index] = value * FILTSTR_FL;   // init empty filter
   filter[index] -= filter[index] / FILTSTR_FL;
   filter[index] += value;
   return (filter[index] / FILTSTR_FL);
}

int16_t Round_filter_int(uint8_t index, int16_t value)      // filtr index
{
   static float filter[MAX_FILTERS_INT];
   if(index > MAX_FILTERS_INT-1) index = MAX_FILTERS_INT-1;  // protection
   if(filter[index] == 0) filter[index] = value * FILTSTR_INT;   // init empty filter
   filter[index] -= filter[index] / FILTSTR_INT;
   filter[index] += value;
   return (filter[index] / FILTSTR_INT);
}

void MCUgoSleep()
{
	stop_meas();
    // prepare wakeup pin
    HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN1);
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
    HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);
    // go sleep
    printf("Sleep\r\n");
    PS_OFF();
    HAL_PWR_EnterSTANDBYMode();
}

void fill_temperature_table()
{
    tlm.temps[0] = Round_filter_fl(1,GET_Buff_Temp());
    tlm.temps[1] = Round_filter_fl(2,LTC4015_get_dietemp());
    tlm.temps[2] = 0; // TODO: NTC Temp
}

void fill_voltage_table()
{
	tlm.volts[0] = GET_SYS_5V();
	tlm.volts[1] = GET_DC_5V();
	tlm.volts[2] = GET_STB_5V();
	tlm.volts[3] = GET_SYS_12V();
	tlm.volts[4] = GET_DC_12V();
	tlm.volts[5] = LTC4015_get_vin();
	tlm.volts[6] = LTC4015_get_vbat(num_cells, chem_type);
}

void fill_current_table()
{
    tlm.curr[0] = Round_filter_int(1,GET_SYS_5V_CURR());
    tlm.curr[1] = Round_filter_int(2,GET_SYS_12V_CURR());
    tlm.curr[2] = Round_filter_int(3,GET_HDD_5V_CURR());
    tlm.curr[3] = Round_filter_int(4,GET_HDD_12V_CURR());
    tlm.curr[4] = Round_filter_int(5,LTC4015_get_iin(RSNI));
    tlm.curr[5] = Round_filter_int(6,LTC4015_get_ibat(RSNB));
}

void print_volt_curr()
{
	printf("---------------------------\r\n");
	printf("Temp round: %3.1f deg C\r\n", tlm.temps[0]);
	printf("5V SYS: %2.2f V ,", tlm.volts[0]);
	printf("5V DC: %2.2f V\r\n", tlm.volts[1]);
	printf("12V SYS: %2.2f V\r\n", tlm.volts[3]);
	printf("5V STB: %2.2f V\r\n", tlm.volts[2]);
	printf("Vin: %2.2f  V , ", tlm.volts[5]);
	printf("Vbat: %2.2f V\r\n", tlm.volts[6]);
	printf("5V SYS Curr : %d mA , ", tlm.curr[0]);
	printf("5V HDD Curr : %d mA\r\n", tlm.curr[2]);
	printf("12V SYS Curr : %d mA , ", tlm.curr[1]);
	printf("12V HDD Curr : %d mA\r\n", tlm.curr[3]);
	printf("IIN Curr : %d mA , ", tlm.curr[4]);
	printf("IBAT Curr : %d mA\r\n", tlm.curr[5]);
	printf("SMBALERT : %d \r\n", tlm.smb);
	printf("DIE TEMP : %2.2f deg C\r\n", tlm.temps[1]);
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

void supply_check_select()
{
	if((tlm.volts[2] <= 5.4f && tlm.volts[2] >= 4.5f) && ps_pg_state == 0 && (run_state == RUNNING_BUFFER || run_state == CHECKING)) ps_count++;
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
		tlm.supply = PS;
		printf("PS select \r\n");
		ledSweepPwr(1,1,1);
		HAL_Delay(100);
		PWR_LED_ON();
	}


	if (ps_pg_state == 0 && (run_state == RUNNING_PS || run_state == CHECKING))
	{
		_5V_SEL_BUF();
		_12V_SEL_BUF();
		run_state = RUNNING_BUFFER;
		tlm.supply = BUFFER;
		printf("Buffer select \r\n");
		HAL_Delay(10);
		ledSweepPwr(20,0xFFFF,30);
	}
}

void check_powerOn()
{
	  POWER_OFF();
	  tlm.supply = INIT;
	  if (__HAL_PWR_GET_FLAG(PWR_FLAG_SB) != RESET)
	  {
	    /* Clear Standby flag */
	    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);
	  }
	  uint32_t timon = HAL_GetTick();
	  while(Power_SW_READ() == GPIO_PIN_SET)
	  {
	    if(HAL_GetTick() - timon > 1000)     // 1 sec pushing
	    {
	    	Buzz(1000);
	    	RPI_POWER_ON();
	    	timon = HAL_GetTick();
	        POWER_ON();    // pull-up power supply
	    	printf("Power ON\r\n");
	    	run_state = CHECKING;
	        break;                // break while loop
	    }

	  //  WDR();    // watchdog reset
	  }
	  if(run_state == WARMUP)
	  {
	      HAL_Delay(300);
	      MCUgoSleep();
	  }
}

void check_powerOff()
{
  if(Power_SW_READ() && rpiTout == 0) //power button pressed
  {
     if(offTim && HAL_GetTick() - offTim > 2000)    // 2 sec pressed
     {
    	 ledSweepStat(3,50,10);
    	 ledSweepPwr(3,50,5);
         Buzz(500);
    	 printf("RPi power down in progress ...\r\n");
         rpiTout = HAL_GetTick();  // mark shutdown time
         RPI_POWER_OFF();           // forced RPi power down
     }
  } else offTim = HAL_GetTick();   // button released, update offTim

  if(rpiTout && HAL_GetTick() - rpiTout > 125000)   // power off in progress and passed 120 sec
  {
      rpiTout = 0;
      stop_meas();
      HAL_Delay(3000);
      POWER_OFF();
      printf("Power OFF\r\n");
      MCUgoSleep();
  }
  if(RPI_FB_READ() == 0 && HAL_GetTick() - rpiTout < 115000)   // RPi confirmed power off and <5 sec
  {
	  printf("RPI Power off Feedback\r\n");
      rpiTout = HAL_GetTick() - 115000;     // 5s to forced power down
  }

  if(rpiTout)
  {
 	  ledSweepPwr(3,0xFFFF,15);
	  ledSweepStat(3,0xFFFF,30);
		if(HAL_GetTick()-beepTim >= 1500)
		{
			beepTim = HAL_GetTick();
		 	Buzz(400);
		}
  }
}

void Load_defaults()
{
	memset((uint8_t*)&config, 0, sizeof(config));
	config.version 				= CONFIG_VERSION;
	config.temp_offset			= -6.0f;
	config.sys5v_offset         = 0.0f;
	config.dc5v_offset          = 0.0f;
	config.stb5v_offset         = 0.0f;
	config.sys12v_offset        = 0.0f;
	config.dc12v_offset         = 0.0f;
	config.curr_sys12v_offset	= -63;
	config.curr_hdd12v_offset	= 32;
	config.curr_sys5v_offset	= -61;
	config.curr_hdd5v_offset	= 34;
}

void Config_init()
{
	switch(Load_config())
	{
    	case 0:
    		printf("Config OK\r\n");
    		break;
    	case 1:
    		printf("Flash read error, Defaults loaded\r\n");
    		break;
    	case 2:
    		printf("Config corrupted or structure changed, Defaults loaded\r\n");
    		break;
    	case 3:
    		printf("Flash write error!\r\n");
    		break;
	}
}

void telemetrySend()
{
   static uint8_t sendbuf[ sizeof(telem_t)];      // send buffer
   static uint32_t sendtim;

   if(HAL_GetTick() - sendtim >= 1000)            // send every 1s
   {
      sendtim = HAL_GetTick();
      memcpy(sendbuf, &tlm, sizeof(telem_t));                            // copy to send buffer, &tlm - struct address
      HAL_UART_Transmit_DMA(&huart3, sendbuf, sizeof(sendbuf));    // send buffer via DMA
  }
}
