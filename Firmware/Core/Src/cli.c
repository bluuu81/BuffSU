/*
 * cli.c
 *
 *  Created on: 05.12.2021
 *      Author: bluuu
 */

static char clibuf[32];
static int cliptr;
#include "cli.h"
#include "ctype.h"
#include <stdio.h>
#include <string.h>

void help(void)
{
	printf("\r\n --CLI-- \r\n");
	printf("?,help - Show help.\r\n");
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
//	char *p;
	if(cliptr < sizeof(clibuf)) clibuf[cliptr++] = ch;
	if(ch == 10)	// LF
	{
	    if(clibuf[cliptr-1] == 13) cliptr--;
		memset(clibuf+cliptr, 0, sizeof(clibuf)-cliptr);
		cliptr = 0;
// Main commands ------------------------------------------------------------------------------
		if(find("?")==clibuf+1 || find("help")==clibuf+2)	{help(); return;}
        // ................................................................................
	}
}
