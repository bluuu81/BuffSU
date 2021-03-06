/*
 * adc.c
 *
 *  Created on: 18.12.2021
 *      Author: bluuu
 */

#include "main.h"
#include "adc.h"
#include "buff.h"
#include <stdio.h>

volatile uint16_t adc_data[50];
uint8_t curr_zero_proc=0;


void ADC_Print()
{
	  printf("ADC 1: %d , ADC 2: %d ADC 3: %d ADC 4: %d ADC 5: %d \r\n", adc_data[0], adc_data[10], adc_data[20], adc_data[30], adc_data[40]);
/*		for(int i=0; i<10; i++)
		{
			printf("ADC %d : %d ", i+1, adc_data[i]);
		}
		printf("\n");
		for(int i=10; i<20; i++)
		{
			printf("ADC %d : %d ", i-9, adc_data[i]);
		}
		printf("\n");
		*/
}


void ADC_DMA_Start()
{
	  HAL_ADCEx_Calibration_Start(&hadc1);                    // ADC calibration
	  HAL_Delay(10);
	  HAL_ADC_Start_DMA(&hadc1, (uint32_t *) adc_data, 50);   // start ADC DMA (10 channels, 5 reads per channel)
}

float GET_Buff_Temp()
{
	    uint16_t tmp=0;
	    float temperature;

	  	for(uint8_t i=0; i<50;i+=10)
	  	{
	  		tmp+=adc_data[i];
	  	}
	  	tmp*=0.2f;

	  	temperature = ((1.43-(tmp*(3.3/4096)))/4.3E-03) + 25.0f;
	  	return temperature+(config.temp_offset);
}


float GET_SYS_5V()
{
	    uint16_t tmp=0;
	    float voltage;

	  	for(uint8_t i=5; i<50;i+=10)
	  	{
	  		tmp+=adc_data[i];
	  	}
	  	tmp*=0.2f;

	  	voltage = tmp * (3.3f/4096) * 1.82f;
	  	return voltage+(config.sys5v_offset);
}

float GET_DC_5V()
{
	    uint16_t tmp=0;
	    float voltage;

	  	for(uint8_t i=8; i<50;i+=10)
	  	{
	  		tmp+=adc_data[i];
	  	}
	  	tmp*=0.2f;

	  	voltage = tmp * (3.3f/4096) * 1.82f;
	  	return voltage+(config.dc5v_offset);
}

float GET_STB_5V()
{
	    uint16_t tmp=0;
	    float voltage;

	  	for(uint8_t i=9; i<50;i+=10)
	  	{
	  		tmp+=adc_data[i];
	  	}
	  	tmp*=0.2f;

	  	voltage = tmp * (3.3f/4096) * 1.82f;
	  	return voltage+(config.stb5v_offset);
}

float GET_SYS_12V()
{
	    uint16_t tmp=0;
	    float voltage;

	  	for(uint8_t i=6; i<50;i+=10)
	  	{
	  		tmp+=adc_data[i];
	  	}
	  	tmp*=0.2f;

	  	voltage = tmp * (3.3f/4096) * 4.6f;
	  	return voltage+(config.sys12v_offset);
}

float GET_DC_12V()
{
	    uint16_t tmp=0;
	    float voltage;

	  	for(uint8_t i=7; i<50;i+=10)
	  	{
	  		tmp+=adc_data[i];
	  	}
	  	tmp*=0.2f;

	  	voltage = tmp * (3.3f/4096) * 4.6f;
	  	return voltage+(config.dc12v_offset);
}

int16_t GET_HDD_12V_CURR()
{
	    int16_t tmp=0;
	    int16_t current;

	  	for(uint8_t i=2; i<50;i+=10)
	  	{
	  		tmp+=adc_data[i];
	  	}
	  	tmp/=5;

	  	current = (((33000U * tmp) / 4096) - 3300) / 2;
	  	if(!curr_zero_proc)
	  	{
	  		current+=(config.curr_hdd12v_offset);
	  		if (current < 0) current = 0;
	  	}
	  	return current; // mA
}

int16_t GET_HDD_5V_CURR()
{
	    int16_t tmp=0;
	    int16_t current;

	  	for(uint8_t i=1; i<50;i+=10)
	  	{
	  		tmp+=adc_data[i];
	  	}
	  	tmp/=5;

	  	current = (((33000U * tmp) / 4096) - 3300) / 2;
	  	if(!curr_zero_proc)
	  	{
	  		current+=(config.curr_hdd5v_offset);
	  		if (current < 0) current = 0;
	  	}
	  	return current; // mA
}

int16_t GET_SYS_12V_CURR()
{
	    int16_t tmp=0;
	    int16_t current;

	  	for(uint8_t i=4; i<50;i+=10)
	  	{
	  		tmp+=adc_data[i];
	  	}
	  	tmp/=5;

	  	current = (((33000U * tmp) / 4096) - 3300) / 2;
	  	if(!curr_zero_proc)
	  	{
	  		current+=(config.curr_sys12v_offset);
	  		if (current < 0) current = 0;
	  	}
	  	return current; // mA
}

int16_t GET_SYS_5V_CURR()
{
	    int16_t tmp=0;
	    int16_t current;

	  	for(uint8_t i=3; i<50;i+=10)
	  	{
	  		tmp+=adc_data[i];
	  	}
	  	tmp/=5;

	  	current = (((33000U * tmp) / 4096) - 3300) / 2;
	  	if(!curr_zero_proc)
	  	{
	  		current+=(config.curr_sys5v_offset);
	  		if (current < 0) current = 0;
	  	}
	  	return current; // mA
}
