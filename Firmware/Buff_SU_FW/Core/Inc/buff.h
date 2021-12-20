/*
 * buff.h
 *
 *  Created on: 19.12.2021
 *      Author: bluuu
 */

#ifndef INC_BUFF_H_
#define INC_BUFF_H_

#include "stm32f1xx_hal.h"
extern uint8_t ps_pg_state, rpi_feedback, smbalert;


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
#endif /* INC_BUFF_H_ */
