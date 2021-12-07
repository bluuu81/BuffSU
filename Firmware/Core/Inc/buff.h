/*
 * buff.h
 *
 *  Created on: 12.09.2021
 *      Author: bluuu
 */

#ifndef INC_BUFF_H_
#define INC_BUFF_H_

#include "stm32f1xx_hal.h"
#include "main.h"
#include <string.h>
#include <math.h>

extern ADC_HandleTypeDef  hadc1;
extern DMA_HandleTypeDef  hdma_adc1;
extern IWDG_HandleTypeDef hiwdg;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;

float Round_filter_fl(uint8_t index, float value);
int16_t Round_filter_int(uint8_t index, int16_t value);
void Buzz(uint32_t tim);
void checkPowerOff();
void MCUgoSleep();
void CLI();


#endif /* INC_BUFF_H_ */
