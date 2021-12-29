/*
 * config.h
 *
 *  Created on: 21.12.2021
 *      Author: bluuu
 */

#ifndef INC_CONFIG_H_
#define INC_CONFIG_H_

#include "stm32f1xx_hal.h"

#define FW_VER 12
#define HW_VER 10

uint8_t Save_config();
uint8_t Load_config();
void Load_defaults();
uint16_t Crc16_up(uint16_t crc, uint8_t data);			// liczenie CRC

void Flash_read(uint32_t adr, uint8_t *data, uint32_t size);           // adr - adres w flash w bajtach 0..524288 bajtow
uint8_t Flash_write_block(uint32_t blk, uint8_t *buff, uint32_t len);

#endif /* INC_CONFIG_H_ */
