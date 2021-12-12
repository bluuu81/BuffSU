/*
 * pwm.h
 *
 *  Created on: Dec 11, 2021
 *      Author: bluuu
 */

#ifndef INC_PWM_H_
#define INC_PWM_H_

void setPwrLed(uint8_t bri);
void setStatLed(uint8_t bri);
void ledSweepPwr(uint16_t spd, uint16_t cnt, uint16_t wait);
void ledSweepStat(uint16_t spd, uint16_t cnt, uint16_t wait);
void Buzz(uint32_t tim);
void PWM_Init_Timers();

#endif /* INC_PWM_H_ */
