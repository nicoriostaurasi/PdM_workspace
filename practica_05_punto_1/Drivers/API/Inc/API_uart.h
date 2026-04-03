/*
 * API_uart.h
 *
 *  Created on: 2 abr 2026
 *      Author: nicol
 */

#ifndef API_INC_API_UART_H_
#define API_INC_API_UART_H_

#include "API_delay.h"

#define UART_MAX_STRING_LENGTH 256
#define UART_MIN_STRING_LENGTH 1
#define UART_MAX_TRANSMIT_ATTEMPTS 10
#define UART_TRANSMIT_TIMEOUT 1000
#define UART_RECEIVE_TIMEOUT 1000

#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA

bool_t uartInit();
void uartSendString(uint8_t * pstring);
void uartSendStringSize(uint8_t * pstring, uint16_t size);
void uartReceiveStringSize(uint8_t * pstring, uint16_t size);


#endif /* API_INC_API_UART_H_ */
