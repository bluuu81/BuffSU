/*
 * ltc4015.h
 *
 *  Created on: 22.09.2021
 *      Author: bluuu
 */

#ifndef INC_LTC4015_H_
#define INC_LTC4015_H_

#include "stm32f1xx_hal.h"

//Read registers
#define VBAT 0x3A // Two's complement ADC measurement result for VIN.
 // VIN = [VIN] x 1.8mV
#define VIN 0x3B // Two's complement ADC measurement result for VIN.
 // VIN = [VIN] x 1.8mV
#define VSYS 0x3C // Two's complement ADC measurement result for VSYS.
 // VSYS = [VSYS] x 1.8mV
#define IBAT 0x3D // Two's complement ADC measurement result for (VCSP -VCSN).
 // Charge current (into the battery) is represented as a positive number.
 // Battery current = [IBAT] x 1.487uV/Rsnsb
#define IIN 0x3E // Two's complement ADC measurement result for (VCLP -VCLN).
 // Input current = [IIN] x 1.487uV/Rsnsi
#define DIE_TEMP 0x3F // Two's complement ADC measurement result for die temperature.
 // Temperature = (DIE_TEMP -12010)/45.6 °C
#define NTC_RATIO 0x40 // Two's complement ADC measurement result for NTC thermistor ratio.
 // Rntc = NTC_RATIO x Rntcbias/(21845.0 -NTC_RATIO)
#define BSR 0x41 // Calculated battery series resistance.
 // For lithium chemistries, series resistance/cellcount = BSR x Rsnsb/500.0
 // For lead-acid chemistries, series resistance/cellcount = BSR x RSNSB/750.0
#define CHEM_CELLS 0x43 // Readout of CHEM and CELLS pin settings
// Config registers

#define VBAT_LO_ALERT_LIMIT 0x01 // Battery voltage low alert limit
#define VBAT_HI_ALERT_LIMIT 0x02 // Battery voltage high alert limit
#define VIN_LO_ALERT_LIMIT 0x03 // Input voltage low alert limit
#define VIN_HI_ALERT_LIMIT 0x04 // Input voltage high alert limit
#define VSYS_LO_ALERT_LIMIT 0x05 // Output voltage low alert limit
#define VSYS_HI_ALERT_LIMIT 0x06 // Output voltage high alert limit
#define IIN_HI_ALERT_LIMIT 0x07 // Input current high alert limit
#define IBAT_LO_ALERT_LIMIT 0x08 // Charge current low alert limit
#define DIE_TEMP_HI_ALERT_LIMIT 0x09 // Die temperature high alert limit
#define BSR_HI_ALERT_LIMIT  0x0A // Battery series resistance high alert limit
#define NTC_RATIO_HI_ALERT_LIMIT 0x0B // Thermistor ratio high (cold battery) alert limit
#define NTC_RATIO_LO_ALERT_LIMIT 0x0C // Thermistor ratio low (hot battery) alert limit

#define ICHARGE_TARGET 0x1A // Maximum charge current target = (ICHARGE_TARGET + 1)x1mV/RSNSB
#define VCHARGE_SETTING 0x1B // Charge voltage target.

//Control bits
#define CONFIG_BITS 0x14 // Configuration Settings
#define suspend_charger (1 << 8)
#define force_meas_sys_on (1 << 4)
#define run_bsr (1 << 5)

extern I2C_HandleTypeDef hi2c1;

//uint8_t i2c_read16(uint16_t offset, uint16_t *value);
//uint8_t i2c_write16(uint16_t offset, uint16_t value);

uint8_t LTC4015_check();
float LTC4015_get_vin();
float LTC4015_get_vbat(uint8_t _cells, uint8_t _chem);
int16_t LTC4015_get_iin(uint8_t rsni);
int16_t LTC4015_get_ibat(uint8_t rsnb);
float LTC4015_get_dietemp();
uint8_t LTC4015_get_cells();
uint8_t LTC4015_get_chem();
void LTC4015_VBAT_limits (float lo, float hi, uint8_t _cells, uint8_t _chem);
void LTC4015_VIN_limits (float lo, float hi);
void LTC4015_set_charge_current(float ch_curr, uint8_t rsnb);
void LTC4015_set_charge_voltage(float ch_volt, uint8_t _cells, uint8_t _chem);
uint16_t read_register_val(uint8_t reg);

void stop_charging();
void start_charging();
void start_meas();
void stop_meas();
void start_run_bsr();
void stop_run_bsr();
void LTC4015_init();

#endif /* INC_LTC4015_H_ */
