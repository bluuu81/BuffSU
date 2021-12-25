/*
 * buff.h
 *
 *  Created on: 19.12.2021
 *      Author: bluuu
 */

#ifndef INC_BUFF_H_
#define INC_BUFF_H_

#include "stm32f1xx_hal.h"
extern uint8_t ps_pg_state, rpi_feedback; //smbalert;

#define CONFIG_VERSION  1

typedef struct {
    uint8_t  version;               // struct ver
    float  temp_offset;             // mcu temperature offset
    float  sys5v_offset;            // System 5V offset
    float  dc5v_offset;             // DC/DC 5V offset
    float  stb5v_offset;            // 5V PS STB offset
    float  sys12v_offset;           // System 12V offset
    float  dc12v_offset;            // DC/DC 12V offset
    int16_t curr_sys12v_offset;		// Curr SYS 12V offset
    int16_t curr_hdd12v_offset;		// Curr HDD 12V offset
    int16_t curr_sys5v_offset;		// Curr SYS 5V offset
    int16_t curr_hdd5v_offset;		// Curr HDD 5V offset
    uint16_t checksum;
}  __attribute__((packed)) config_t;

typedef struct {
	float		temps[3];
	float		volts[7];
	int16_t		curr[6];
	uint16_t	supply:2;
	uint16_t	smb:1;
	uint16_t	limits:4;
	uint16_t	charge:2;
	uint16_t	resv:7;
} __attribute__((packed)) telem_t;

enum tlm_supply_state {
	INIT = 0,
	BUFFER,
	PS,
	FAIL,
};

config_t config;
telem_t tlm;

float Round_filter_fl(uint8_t index, float value);
int16_t Round_filter_int(uint8_t index, int16_t value);
void MCUgoSleep();
void BUFF_fill_values();
void fill_temperature_table();
void fill_voltage_table();
void fill_current_table();

void print_volt_curr();
void print_regs();

void supply_check_select();
void check_powerOn();
void check_powerOff();
void Load_defaults();
void Config_init();
void telemetrySend();

#endif /* INC_BUFF_H_ */
