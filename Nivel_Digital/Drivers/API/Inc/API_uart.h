/*
 * API_uart.h
 *
 *  Created on: 2 abr 2026
 *      Author: nicol
 */

#ifndef API_INC_API_UART_H_
#define API_INC_API_UART_H_

#include "API_delay.h"

// Definiciones de constantes para la UART
#define UART_MAX_STRING_LENGTH 256
#define UART_MIN_STRING_LENGTH 1
#define UART_MAX_TRANSMIT_ATTEMPTS 10
#define UART_TRANSMIT_TIMEOUT 1000
#define UART_RECEIVE_TIMEOUT 1000

// Pines de UART
#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA

/** @brief Función para inicializar la UART
  * @return: true si la inicialización fue exitosa, false en caso contrario
  */
bool_t uart_init(void);

/** @brief Función para enviar una cadena de caracteres por UART
  * @param pstring: Puntero al string a enviar
  */
void uart_sendString(uint8_t * pstring);

/** @brief Función para enviar una cadena de caracteres por UART con un tamaño específico
  * @param pstring: Puntero al string a enviar
  * @param size: Tamaño del string a enviar
  */
void uart_sendStringSize(uint8_t * pstring, uint16_t size);

/** @brief Función para recibir una cadena de caracteres por UART con un tamaño específico
  * @param pstring: Puntero al buffer donde se almacenará la cadena recibida
  * @param size: Tamaño del string a recibir
  */
void uart_receiveStringSize(uint8_t * pstring, uint16_t size);

/** @brief Función para saber si hubo una lectura exitosa
 *  @return: true si hay nuevos datos, false en caso contrario
 */
bool uart_isNewDataOnRx(void);

/** @brief Función para obtener la tasa de baudios actual
 *  @return: La tasa de baudios actual
 */
uint32_t uart_getCurrentBaudrate(void);

/** @brief Función para cambiar la tasa de baudios
 *  @param newBaudrate: La nueva tasa de baudios a configurar
 *  @return: true si el cambio fue exitoso, false en caso contrario
 */
bool_t uart_changeCurrentBaudrate(uint32_t newBaudrate);

bool_t uart_rxPop(uint8_t *data);

void uart_setLoopback(bool_t enable);

bool_t uart_getLoopback(void);


#endif /* API_INC_API_UART_H_ */
