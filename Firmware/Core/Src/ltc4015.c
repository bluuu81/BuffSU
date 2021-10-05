/*
 * ltc4015.c
 *
 *  Created on: 22.09.2021
 *      Author: bluuu
 */
#include "ltc4015.h"
#include "main.h"
#include <stdio.h>

#define LTC_ADDR 0xD0

uint8_t i2c_read16(uint16_t offset, uint16_t *value)
{
	uint16_t tmp;
    uint8_t res = HAL_I2C_Mem_Read(&hi2c1, LTC_ADDR, offset, I2C_MEMADD_SIZE_8BIT, (uint8_t*)&tmp, 2, 8);
    *value = tmp;
    return res;
}

uint8_t i2c_write16(uint16_t offset, uint16_t value)
{
	uint16_t tmp = value;
    uint8_t res = HAL_I2C_Mem_Write(&hi2c1, LTC_ADDR, offset, I2C_MEMADD_SIZE_8BIT, (uint8_t*)&tmp, 2, 8);
    return res;
}


// Checking
uint8_t LTC4015_check()
{
	HAL_StatusTypeDef status;
	uint8_t res;
	printf("Checking LTC4015 ... ");
	for (int i = 0; i < 10; i++) {
		status = HAL_I2C_IsDeviceReady(&hi2c1, LTC_ADDR, 3, 1500);
		HAL_Delay(100);
	    if (status == HAL_OK) {
	    	printf("OK !\r\n");
	    	res = 1;
	        break;
	    } else {
	    	printf("not ready\r\n");
	    	res = 0;
	    }
	}
	return res;
}

// LTC4015 configuration

void LTC4015_VBAT_limits (uint16_t hi, uint16_t lo)
{
	i2c_write16(VBAT_LO_ALERT_LIMIT, lo);
	i2c_write16(VBAT_HI_ALERT_LIMIT, hi);
}


// LTC4015 read registers

uint16_t LTC4015_get_vin()
{
    uint16_t status;
    if(i2c_read16(VIN, &status)) return -1;;
    return status;
}

void LTC4015_init()
{
	LTC4015_VBAT_limits(0,0);
}
