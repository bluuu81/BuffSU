/*
 * adc.h
 *
 *  Created on: 18.09.2021
 *      Author: bluuu
 */

#ifndef INC_ADC_H_
#define INC_ADC_H_

void ADC_DMA_Start();
void ADC_Print();

float GET_Buff_Temp();
float GET_SYS_5V();
float GET_DC_5V();
float GET_STB_5V();
float GET_SYS_12V();
float GET_DC_12V();
int16_t GET_HDD_12V_CURR();
int16_t GET_HDD_5V_CURR();
int16_t GET_SYS_12V_CURR();
int16_t GET_SYS_5V_CURR();

#endif /* INC_ADC_H_ */
