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

extern I2C_HandleTypeDef hi2c1;

//uint8_t i2c_read16(uint16_t offset, uint16_t *value);
//uint8_t i2c_write16(uint16_t offset, uint16_t value);

uint8_t LTC4015_check();
uint16_t LTC4015_get_vin();
void LTC4015_init();

#endif /* INC_LTC4015_H_ */
