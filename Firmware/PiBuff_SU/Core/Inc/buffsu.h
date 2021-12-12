/*
 * buffsu.h
 *
 *  Created on: Dec 11, 2021
 *      Author: bluuu
 */

#ifndef INC_BUFFSU_H_
#define INC_BUFFSU_H_

extern ADC_HandleTypeDef  hadc1;
extern DMA_HandleTypeDef  hdma_adc1;
extern IWDG_HandleTypeDef hiwdg;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;
extern TIM_HandleTypeDef  htim2;

//switch pwm/gpio led signaling
// #define GPIO 1

#include "stm32f1xx_hal.h"
#include "main.h"
#include <string.h>
#include <math.h>

void startup_check();
void checkPowerOff();
void MCUgoSleep();
void BuffSU_Loop();

#endif /* INC_BUFFSU_H_ */
