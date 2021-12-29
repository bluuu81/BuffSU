/*
 * cli.c
 *
 *  Created on: 18.12.2021
 *      Author: bluuu
 */

#include "cli.h"
#include "main.h"
#include "config.h"
#include "ltc4015.h"
#include "ctype.h"
#include <stdio.h>
#include <string.h>

#define RX_BUF_SIZE 512

uint8_t  uart_rx_buf[RX_BUF_SIZE];
uint16_t uart_rxtail;

uint8_t telem_rx_buf[32];    // 32 bytes buffer
uint16_t telem_rxtail;

static char clibuf[32];
static int cliptr;

uint8_t telem_run = 0;


// printf redirection to UART1
int _write(int file, char *ptr, int len)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)ptr, len, len+1);  // uart1
    return len;
}

void debug_putchar(uint8_t ch)
{
    HAL_UART_Transmit(&huart1, &ch, 1, 2);  // debug uart
}

// Debug UART RX

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart == &huart1) HAL_UART_Receive_IT(&huart1, uart_rx_buf, RX_BUF_SIZE);  // Interrupt start Uart1 RX
	if(huart == &huart3) HAL_UART_Receive_IT(&huart3, telem_rx_buf, 32); // Interrupt start Uart3 RX
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    HAL_UART_RxCpltCallback(huart);
}

uint16_t UART_has_char()      // Return number of bytes in buffer
{
    return (huart1.RxXferSize-huart1.RxXferCount+RX_BUF_SIZE-uart_rxtail) % RX_BUF_SIZE;
}

uint8_t UART_receive()       // Receive byte from buffer
{
    uint8_t tmp = uart_rx_buf[uart_rxtail++];
    if(uart_rxtail >= RX_BUF_SIZE) uart_rxtail = 0;
    return tmp;
}

void CLI() {
    int len = UART_has_char();
    if(len) { for(int i=0; i<len; ++i) CLI_proc(UART_receive()); }
}

uint16_t TELEM_has_char() // Return number of bytes in telem buffer
{
    return (huart3.RxXferSize-huart3.RxXferCount+32-telem_rxtail) % 32;
}

uint8_t TELEM_receive() // Receive byte from telem buffer
{
   uint8_t tmp = telem_rx_buf[telem_rxtail++];
   if(telem_rxtail >= 32) telem_rxtail = 0;
   return tmp;
}

void telemetryRX()
{
   static uint8_t state;
   while( TELEM_has_char() )
   {
       uint8_t ch = TELEM_receive();
       switch(state)
       {
          case 0: if(ch == 0xBA)	state = 1;
                      break;
          case 1: if(ch == 0xDA)	state = 2;
                      if(ch != 0xDA) state = 0;
                      break;
          case 2: if(ch == 0xBE) state = 3;
                      if(ch != 0xBE) state = 0;
                      break;
          case 3: if(ch == 0xAF) state = 4;
                      if(ch != 0xAF) state = 0;
                      break;
          case 4: if(ch == 0xAA) { telem_run = 1; printf("Telem ON\r\n"); }
                      if(ch == 0xFF) { telem_run = 0; printf("Telem OFF\r\n"); }
                      state = 0;
                      break;
       }
   }
}


void help()
{
	printf("\r\n--- CLI Buff SU HW v%1.1f, FW v%1.1f ---\r\n", HW_VER/10.0f, FW_VER/10.0f);
	printf("?,help - Show help\r\n");
	printf("debug 0..2      0 - off, 1 - LTC4015 registers, 2 - voltage & current\r\n");
	printf("charger stop/start\r\n");
	printf("load defaults\r\n");
	printf("-------\r\n");
}

char * find(const char *arg2)							// find token in cmdline
{
	int i,j,k;
	for(i=0; clibuf[i]; i++)
		for(j=i, k=0; tolower(clibuf[j]) == arg2[k]; j++, k++)
			if(!arg2[k+1]) return (clibuf+(j+1));
	return NULL;
}

char * getval(char *p, int32_t *val, int32_t min, int32_t max)		// get s32 value from cmdline
{
	int32_t tmp = 0;
	if(*p == ' ') p++;
	uint8_t sign = (*p == '-') ? 1:0;
	while(*p)
	{
		if(*p >= '0' && *p <= '9') {tmp *= 10; tmp += *p - '0'; p++;}
		else break;
	}
	if(sign) tmp = -tmp;
	if(tmp >= min && tmp <= max) *val = tmp; else printf("Bad value\r\n");
	return p;
}

void getString(char *p, char *dst, int16_t minlen, int16_t maxlen, const char *nam)		// get string from cmdline
{
	if(*p == ' ') p++;
	for(int i=0; i<maxlen; i++) if(p[i]==13 && i<minlen) {printf("Too short\r\n"); return;}	// test dlugosci
	for(int i=0; i<maxlen; i++)
	{
		dst[i] = p[i];
		if(p[i] == 13) {dst[i] = 0; break;}
	}
	printf("%s: %s\r\n", nam, dst);
}

char * get_hex(char *p, uint8_t chars, uint16_t *val)
{
    uint16_t tmp = 0;
    if(*p==' ' || *p==',') p++;
    while(*p)
    {
        if(*p == 13 || *p == 10) {p = NULL; break;}
        if(*p==' ' || *p==',') break;
        char asc = tolower(*p);
        if(asc >= '0' && asc <= '9')      {tmp *= 16; tmp += asc - '0'; p++;}
        else if(asc >= 'a' && asc <= 'f') {tmp *= 16; tmp += asc - 'a'+10; p++;}
        else p++;
        if(--chars == 0) break;
    }
    *val = tmp;
    return p;
}

void CLI_proc(char ch)
{
	char *p;
	if(cliptr < sizeof(clibuf)) clibuf[cliptr++] = ch;
	if(ch == 10)	// LF
	{
	    if(clibuf[cliptr-1] == 13) cliptr--;
		memset(clibuf+cliptr, 0, sizeof(clibuf)-cliptr);
		cliptr = 0;
// Main commands ------------------------------------------------------------------------------
		if(find("?")==clibuf+1 || find("help")==clibuf+4)	{help(); return;}
		if(find("load defaults")==clibuf+13)
		{
			Load_defaults();
			Save_config();
			printf("return to defaults ...\r\n");
			return;
		}
// ................................................................................
        if((p = find("debug ")))
        {
            int32_t tmp = -1;
            getval(p, &tmp, 0, 2);
            if(tmp >= 0)
            {
                debug_level = tmp;
                printf("Debug: %u\r\n", debug_level);
            }
            return;
        }

        p = find("charger ");           // set commands
        if(p == clibuf+8)
        {
            if((p = find("start")))
            {
            	printf("Start charging\r\n");
            	start_charging();
                return;
            }
            if((p = find("stop")))
            {
            	printf("Stop charging\r\n");
            	stop_charging();
                return;
            }

        }
	}
}

