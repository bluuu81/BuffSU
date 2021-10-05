/*
 * adc.c
 *
 *  Created on: 18.09.2021
 *      Author: bluuu
 */
#include "buff.h"
#include <stdio.h>

//TODO - do eepromu, tzn. NVR
#define TEMP_OFFSET -9.0f;
#define SYS5V_OFFSET 0;
#define DC5V_OFFSET 0;
#define STB5V_OFFSET 0;
#define SYS12V_OFFSET 0;
#define DC12V_OFFSET 0;

volatile uint16_t adc_data[50];

volatile float filt=0;

void ADC_DMA_Start()
{
	  HAL_ADCEx_Calibration_Start(&hadc1);                    // kalibracja ADC
	  HAL_Delay(10);
	  HAL_ADC_Start_DMA(&hadc1, (uint32_t *) adc_data, 50);   // start ADC DMA (dla 10 kanalow po 1 odczycie na kana³)
}



void ADC_Print()
{
//	  printf("ADC 1: %d , ADC 2: %d \n", adc_data[0], adc_data[1]);
		for(int i=0; i<10; i++)
		{
			printf("ADC %d : %d ", i+1, adc_data[i]);
		}
		printf("\n");
		for(int i=10; i<20; i++)
		{
			printf("ADC %d : %d ", i-9, adc_data[i]);
		}
		printf("\n");
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
	  	return temperature+TEMP_OFFSET;

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
	  	return voltage+SYS5V_OFFSET;
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
	  	return voltage+DC5V_OFFSET;
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
	  	return voltage+STB5V_OFFSET;
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
	  	return voltage+SYS12V_OFFSET;
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
	  	return voltage+DC12V_OFFSET;
}

uint16_t GET_HDD_12V_CURR()
{
	    uint16_t tmp=0;
	    uint16_t current;

	  	for(uint8_t i=2; i<50;i+=10)
	  	{
	  		tmp+=adc_data[i];
	  	}
	  	tmp/=5;

	  	current = (((33000U * tmp) / 4096) - 3300) / 2;
	  	return current; // mA
}

uint16_t GET_HDD_5V_CURR()
{
	    uint16_t tmp=0;
	    uint16_t current;

	  	for(uint8_t i=1; i<50;i+=10)
	  	{
	  		tmp+=adc_data[i];
	  	}
	  	tmp/=5;

	  	current = (((33000U * tmp) / 4096) - 3300) / 2;
	  	return current; // mA
}

uint16_t GET_SYS_12V_CURR()
{
	    uint16_t tmp=0;
	    uint16_t current;

	  	for(uint8_t i=4; i<50;i+=10)
	  	{
	  		tmp+=adc_data[i];
	  	}
	  	tmp/=5;

	  	current = (((33000U * tmp) / 4096) - 3300) / 2;
	  	return current; // mA
}

uint16_t GET_SYS_5V_CURR()
{
	    uint16_t tmp=0;
	    uint16_t current;

	  	for(uint8_t i=3; i<50;i+=10)
	  	{
	  		tmp+=adc_data[i];
	  	}
	  	tmp/=5;

	  	current = (((33000U * tmp) / 4096) - 3300) / 2;
	  	return current; // mA
}


