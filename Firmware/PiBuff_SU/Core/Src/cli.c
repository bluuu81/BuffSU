/*
 * cli.c
 *
 *  Created on: 12 gru 2021
 *      Author: bluuu
 */

#include "main.h"

#define RX_BUF_SIZE 512
uint8_t  uart_rx_buf[RX_BUF_SIZE];
uint16_t uart_rxtail;

// uart functions

// printf redirection to UART1
int _write(int file, char *ptr, int len)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)ptr, len, len+1);  // debug uart
    return len;
}

void debug_putchar(uint8_t ch)
{
    HAL_UART_Transmit(&huart1, &ch, 1, 2);  // debug uart
}

// UART recive

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    HAL_UART_Receive_IT(&huart1, uart_rx_buf, RX_BUF_SIZE);  // Interrupt UART RX start
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    HAL_UART_RxCpltCallback(huart);
}

uint16_t UART_has_char()      // return number of bytes recived
{
    return (huart1.RxXferSize-huart1.RxXferCount+RX_BUF_SIZE-uart_rxtail) % RX_BUF_SIZE;
}

uint8_t UART_receive()       // get byte from buffer
{
    uint8_t tmp = uart_rx_buf[uart_rxtail++];
    if(uart_rxtail >= RX_BUF_SIZE) uart_rxtail = 0;
    return tmp;
}
