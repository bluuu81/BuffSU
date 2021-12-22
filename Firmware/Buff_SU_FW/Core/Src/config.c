/*
 * config.c
 *
 *  Created on: 21.12.2021
 *      Author: bluuu
 */

#include "config.h"
//#include "adc.h"
#include "main.h"
#include "buff.h"
#include <stdio.h>
#include <string.h>

#define CONFIG_VERSION  1
#define ADDR_FLASH_PAGE_0   ((uint32_t)0x08000000) /* Base @ of Page 0, 1 Kbytes */

uint16_t Crc16_up(uint16_t crc, uint8_t data)			// calculate CRC
{
	uint8_t x = crc >> 8 ^ data;
	x ^= x>>4;
	crc = (crc << 8) ^ ((uint16_t)(x << 12)) ^ ((uint16_t)(x <<5)) ^ ((uint16_t)x);
	return crc;
}

void Calc_config_crc(void)		// update CRC of config structure
{
	config.checksum = 0xFFFF;
	uint8_t *p = (uint8_t*)&config;
	for(int i=0; i<sizeof(config)-2; ++i) config.checksum = Crc16_up(config.checksum, *p++);
}

// ---------------------------

uint8_t Flash_write_block(uint32_t blk, uint8_t *buff, uint32_t len)
{
    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t PAGEError = 0;
    uint32_t data32;
//    WDR();
    blk *= FLASH_PAGE_SIZE;
    blk += ADDR_FLASH_PAGE_0;

    HAL_FLASH_Unlock();
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_BSY | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);
    EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.Banks       = FLASH_BANK_1;
    EraseInitStruct.PageAddress = blk;                                      // 2kb page
    EraseInitStruct.NbPages     = 1;
    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK)          // kasuj cala strone
    {
        HAL_FLASH_Lock();           // blad kasowania
        printf("Erase error, page %u\r\n", (int)(blk));
        return 1;
    }
    for(uint32_t i=0; i<len/4 + 1; ++i)          // zapisuj po 4 bajtow, o 8 bajtow wiecej niz trzeba
    {
        data32 = *(uint32_t*)buff;                       // pobierz kolejne 4 bajty z bufora
        buff += 4;                                  // wskaznik bufora zwieksz o 4
        if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, 4*i + blk, data32) != HAL_OK)    // zapisz 8 bajtow do flasha
        {
            HAL_FLASH_Lock();
//            WDR();
            printf("Programming error at %X\r\n",(unsigned int)(4*i + blk));
            return 1;                               // blad
        }
    }
    HAL_FLASH_Lock();
//    WDR();
    return 0;                                       // OK
}

void Flash_read(uint32_t adr, uint8_t *data, uint32_t size)
{
    uint8_t *psrc = (uint8_t*)(ADDR_FLASH_PAGE_0 + adr);
    for(uint32_t i=0; i<size; ++i) data[i] = *psrc++;
}


// SAVE / READ config into FLASH pages 62 and 63 (EEPROM emulation)


#define USE_FLASH_PAGE      62

uint8_t FLASH_write_block(uint32_t adr, uint8_t *buff, uint32_t len)
{
	uint8_t err;
	adr &= 1;				// adr = 0 -> page 62, adr = 1 -> page 63
	adr += USE_FLASH_PAGE;
    for(int i=0; i<4; ++i)
    {
        err =  Flash_write_block(adr, buff, len);
        if(err == 0) break;
    }
	return err;
}

void FLASH_read_block(uint32_t adr, uint8_t *data, uint32_t size)
{
    adr &= 1;               // adr = 0 -> page 62, adr = 1 -> page 63
    adr *= FLASH_PAGE_SIZE;
    adr += (FLASH_PAGE_SIZE * USE_FLASH_PAGE);
    Flash_read(adr, data, size);
}

uint8_t Save_config()
{
	Calc_config_crc();
	uint8_t *p = (uint8_t*)&config;
	uint8_t err = FLASH_write_block(0, p, sizeof(config));		// zapisz do 1 kopii
	err   |= FLASH_write_block(1, p, sizeof(config));		// zapisz do 2 kopii
	return err;			// 0=OK, 1=blad
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
	config.curr_sys12v_offset	= -83;
	config.curr_hdd12v_offset	= 62;
	config.curr_sys5v_offset	= -81;
	config.curr_hdd5v_offset	= 57;
}

uint8_t Load_config()
{
	uint32_t eepok = 0;
	uint32_t len = sizeof(config);
	uint8_t *p = (uint8_t*)&config;
	FLASH_read_block(0, p, len);								// odczyt pierwszej kopii
	uint16_t chk = 0xFFFF;
	for(int i=0; i<len-2; i++) chk = Crc16_up(chk, *p++);
	if(chk == config.checksum) eepok |= 1;						// zaznacz ze jest ok

	p = (uint8_t*)&config;
	FLASH_read_block(1, p, len);								// odczyt drugiej kopii
	chk = 0xFFFF;
	for(int i=0; i<len-2; i++) chk = Crc16_up(chk, *p++);
	if(chk == config.checksum) eepok |= 2;						// zaznacz ze jest ok

	if(config.version != CONFIG_VERSION) eepok = 0;				// zmiana struktury -> laduj defaulty

	p = (uint8_t*)&config;
	switch(eepok)
	{
		case 0:													// obie kopie zwalone
			Load_defaults();
			eepok = Save_config();
			if(eepok) return 3; else return 2;					// 2=zaladowano defaulty, 3=blad zapisu flash
			break;
		case 1:													// 1 ok, 2 zwalona
			FLASH_read_block(0, p, len);
			eepok = FLASH_write_block(1, p, len);				// przepisz do 2 kopii
			if(eepok) return 3;
			break;
		case 2:													// 2 ok, 1 zwalona
			eepok = FLASH_write_block(0, p, len);				// przepisz do 1 kopii
			if(eepok) return 3;
			break;
	}
	return 0;													// config OK
}
