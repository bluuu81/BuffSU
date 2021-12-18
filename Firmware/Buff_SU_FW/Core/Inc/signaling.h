/*
 * signaling.h
 *
 *  Created on: 18.12.2021
 *      Author: bluuu
 */

#ifndef INC_SIGNALING_H_
#define INC_SIGNALING_H_

#include "main.h"

void setPwrLed(uint8_t bri);
void setStatLed(uint8_t bri);
void ledSweepPwr(uint16_t spd, uint16_t cnt, uint16_t wait);
void ledSweepStat(uint16_t spd, uint16_t cnt, uint16_t wait);
void PWM_Init_Timers();
void Buzz(uint32_t tim);

#endif /* INC_SIGNALING_H_ */
