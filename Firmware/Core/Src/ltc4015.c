/*
 * ltc4015.c
 *
 *  Created on: 22.09.2021
 *      Author: bluuu
 */
#include "ltc4015.h"
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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

void clr_bit(unsigned char sub_address, unsigned short new_word) {
    unsigned short old_word;
    i2c_read16(sub_address, &old_word);
    old_word &= ~new_word;
    i2c_write16(sub_address, old_word);
}

void set_bit(unsigned char sub_address, unsigned short new_word) {
    unsigned short old_word;
    i2c_read16(sub_address, &old_word);
    old_word |= new_word;
    i2c_write16(sub_address, old_word);
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

void LTC4015_VBAT_limits (float lo, float hi, uint8_t _cells, uint8_t _chem)
{
	uint16_t val_l,val_h;
	float chem_const;

	if(_chem >= 0 && _chem <=6) chem_const = 0.000192264f;
	else chem_const = 0.000128176f;

	val_l = (uint16_t)(lo / (_cells * chem_const));
	val_h = (uint16_t)(hi / (_cells * chem_const));

	printf("VBAT LO: %d   VBAT HI: %d \r\n", val_l, val_h);
	i2c_write16(VBAT_LO_ALERT_LIMIT, val_l);
	i2c_write16(VBAT_HI_ALERT_LIMIT, val_h);
}

void LTC4015_VIN_limits (float lo, float hi)
{
	uint16_t val_l,val_h;

	val_l = (uint16_t)(lo / 0.001648f);
	val_h = (uint16_t)(hi / 0.001648f);

	printf("VIN LO: %d   VIN HI: %d \r\n", val_l, val_h);
	i2c_write16(VIN_LO_ALERT_LIMIT, val_l);
	i2c_write16(VIN_HI_ALERT_LIMIT, val_h);
}

void LTC4015_set_charge_voltage(float ch_volt, uint8_t _cells, uint8_t _chem)
{
	uint8_t val;
	float ch_cell;

	ch_cell = ch_volt/_cells;

	if(_chem >= 0 && _chem <=3)
		{
			ch_cell -= 3.8125f;
			val = (uint8_t)(ch_cell * 80);
		}
	else if (_chem >= 4 && _chem <=6)
		{
			ch_cell -= 3.4125f;
			val = (uint8_t)(ch_cell * 80);
		}
	else
		{
			ch_cell -= 2.0;
			val = (uint8_t)(ch_cell * 105);
		}
	i2c_write16(VCHARGE_SETTING, val);
	printf("Vcharge : %d \r\n", val);
}

void LTC4015_set_charge_current(float ch_curr, uint8_t rsnb)
{
	uint8_t val;
	val = (uint8_t)((ch_curr * rsnb) - 1);
	i2c_write16(ICHARGE_TARGET, val);
	printf("Icharge : %d \r\n", val);
}

// Control bits
inline void stop_charging()  	{ set_bit(CONFIG_BITS, suspend_charger); }
inline void start_charging()    { clr_bit(CONFIG_BITS, suspend_charger); }

inline void start_meas()		{ set_bit(CONFIG_BITS, force_meas_sys_on); }
inline void stop_meas()			{ clr_bit(CONFIG_BITS, force_meas_sys_on); }

inline void start_run_bsr()		{ set_bit(CONFIG_BITS, run_bsr); }
inline void stop_run_bsr()		{ clr_bit(CONFIG_BITS, run_bsr); }

// LTC4015 read registers

uint16_t read_register_val(uint8_t reg)
{
	uint16_t value;
	if(i2c_read16(reg, &value)) return -1;
	printf("REG : %#04X , Value: %d \r\n", reg, value);
    return value;
}

float LTC4015_get_vin()
{
    uint16_t value;
    float val;
    if(i2c_read16(VIN, &value)) return -1;
    val = value * 0.001648f;
    return (float)val;
}


float LTC4015_get_vbat(uint8_t _cells, uint8_t _chem)
{
    uint16_t value;
    float val;
	float chem_const;
	if(_chem >= 0 && _chem <=6) chem_const = 0.000192264f;
	else chem_const = 0.000128176f;
    if(i2c_read16(VBAT, &value)) return -1;
    value *= _cells;
    val = value * chem_const;
    return (float)val;
}

int16_t LTC4015_get_iin(uint8_t rsni)
{
	uint16_t value;
	int16_t curr;
	if(i2c_read16(IIN, &value)) return -1;
	curr = round((int16_t)value * 1.46487f) / rsni;
	return curr;
}

int16_t LTC4015_get_ibat(uint8_t rsnb)
{
	uint16_t value;
	int16_t curr;
	if(i2c_read16(IBAT, &value)) return -1;
	curr = round((int16_t)value * 1.46487f) / rsnb;
	return curr;
}

float LTC4015_get_dietemp()
{
	uint16_t value;
	float temp;
	if(i2c_read16(DIE_TEMP, &value)) return -1;
	temp = (value - 12010) / 45.6f;
	return temp;
}


uint8_t LTC4015_get_cells()
{
	uint16_t tmp;
	i2c_read16(CHEM_CELLS, &tmp);
	uint8_t cell_count = tmp & 0x0F;
	return cell_count;
}

uint8_t LTC4015_get_chem()
{
	uint16_t tmp;
	uint8_t chem;
	i2c_read16(CHEM_CELLS, &tmp);
	chem = (tmp >> chem) & 0x0F;
	return chem;
}

void LTC4015_init()
{
	start_meas();
	HAL_Delay(15);
//	LTC4015_VBAT_limits(0,0);
}
