/*
 * cli.h
 *
 *  Created on: 18.12.2021
 *      Author: bluuu
 */

#ifndef INC_CLI_H_
#define INC_CLI_H_

#include "stm32f1xx_hal.h"

volatile uint8_t debug_level;

void CLI();
void CLI_proc(char ch);

#endif /* INC_CLI_H_ */
