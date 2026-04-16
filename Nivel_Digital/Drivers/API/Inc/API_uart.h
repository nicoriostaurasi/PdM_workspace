/**
 * @file    API_uart.h
 * @brief   Driver de UART2 con transmisión/recepción por interrupciones y FIFO.
 *
 * @date    2 abr 2026
 * @author  nicol
 */

#ifndef API_INC_API_UART_H_
#define API_INC_API_UART_H_

#include "API_delay.h"

#define UART_MAX_STRING_LENGTH      256     /**< Longitud máxima de cadena a transmitir */
#define UART_MIN_STRING_LENGTH      1       /**< Longitud mínima válida */
#define UART_MAX_TRANSMIT_ATTEMPTS  10      /**< Reintentos de envío ante FIFO llena */
#define UART_TRANSMIT_TIMEOUT       1000    /**< Timeout de transmisión, en ms */
#define UART_RECEIVE_TIMEOUT        1000    /**< Timeout de recepción bloqueante, en ms */

#define USART_TX_Pin        GPIO_PIN_2      /**< Pin TX de USART2 */
#define USART_TX_GPIO_Port  GPIOA           /**< Puerto TX de USART2 */
#define USART_RX_Pin        GPIO_PIN_3      /**< Pin RX de USART2 */
#define USART_RX_GPIO_Port  GPIOA           /**< Puerto RX de USART2 */

/**
 * @brief   Inicializa USART2 a 115200-8N1 y arranca la recepción por interrupción.
 *
 * @return  true si la inicialización fue exitosa, false en caso contrario.
 */
bool_t uart_init(void);

/**
 * @brief   Envía una cadena terminada en '\0' por UART (no bloqueante, vía FIFO TX).
 *
 * @param   pstring     Puntero al string a enviar.
 */
void uart_sendString(uint8_t * pstring);

/**
 * @brief   Envía exactamente @p size bytes por UART (no bloqueante, vía FIFO TX).
 *
 * @param   pstring     Puntero al buffer a enviar.
 * @param   size        Cantidad de bytes a transmitir.
 */
void uart_sendStringSize(uint8_t * pstring, uint16_t size);

/**
 * @brief   Recibe exactamente @p size bytes por UART (bloqueante).
 *
 * @param   pstring     Buffer de destino para los datos recibidos.
 * @param   size        Cantidad de bytes a recibir.
 */
void uart_receiveStringSize(uint8_t * pstring, uint16_t size);

/**
 * @brief   Indica si se recibieron nuevos datos en la última recepción bloqueante.
 *
 * @return  true si hay datos nuevos (resetea el flag internamente), false en
 *          caso contrario.
 */
bool uart_isNewDataOnRx(void);

/**
 * @brief   Devuelve el baudrate actualmente configurado.
 *
 * @return  Baudrate en bps (ej: 115200).
 */
uint32_t uart_getCurrentBaudrate(void);

/**
 * @brief   Reconfigura la UART con un nuevo baudrate.
 *
 * @param   newBaudrate     Nuevo baudrate a configurar (ej: 9600, 115200).
 * @return  true si se aplicó correctamente, false en caso contrario.
 */
bool_t uart_changeCurrentBaudrate(uint32_t newBaudrate);

/**
 * @brief   Extrae un byte de la FIFO de recepción.
 *
 * @param   data    Puntero al byte de salida.
 * @return  true si había un byte disponible, false si la FIFO estaba vacía.
 */
bool_t uart_rxPop(uint8_t *data);

/**
 * @brief   Habilita o deshabilita el eco (loopback) de RX hacia TX.
 *
 * @param   enable  true para habilitar, false para deshabilitar.
 */
void uart_setLoopback(bool_t enable);

/**
 * @brief   Consulta si el eco (loopback) está habilitado.
 *
 * @return  true si está habilitado, false en caso contrario.
 */
bool_t uart_getLoopback(void);


#endif /* API_INC_API_UART_H_ */
