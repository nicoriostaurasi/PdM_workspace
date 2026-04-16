/**
 * @file    API_cmdParser.h
 * @brief   Parser de comandos por UART con FSM no bloqueante y FIFO de salida.
 *
 * @date    3 abr 2026
 * @author  nicol
 */

#ifndef API_INC_API_CMDPARSER_H_
#define API_INC_API_CMDPARSER_H_

#include "API_delay.h"

#define CMD_MAX_LINE    64      /**< Largo máximo de una línea de comando */
#define CMD_MAX_TOKENS  3       /**< Cantidad máxima de tokens por línea */
#define CMD_FIFO_SIZE   4       /**< Capacidad de la FIFO de comandos pendientes */

/**
 * @brief   Resultado del procesamiento de una línea de comando.
 */
typedef enum {
	CMD_OK = 0,                 /**< Comando reconocido y encolado */
	CMD_ERR_OVERFLOW,           /**< Línea demasiado larga o FIFO llena */
	CMD_ERR_SYNTAX,             /**< Línea vacía u otra falla de sintaxis */
	CMD_ERR_UNKNOWN,            /**< Comando no reconocido */
	CMD_ERR_ARG,                /**< Argumento inválido para el comando */
} cmd_status_t;

/**
 * @brief   Identificadores de comandos disponibles.
 */
typedef enum{
	CMD_HELP = 0,               /**< Muestra la ayuda */
	CMD_READ_ANGLE,             /**< Lee el ángulo actual del sensor */
	CMD_READ_ACCELERATION,      /**< Lee la aceleración actual del sensor */
	CMD_STATUS,                 /**< Reporta el estado de salud del sistema */
	CMD_MODE_GET,               /**< Consulta el modo de visualización actual */
	CMD_MODE_TOGGLE,            /**< Alterna entre modo digital y analógico */
	CMD_MODE_DIGITAL,           /**< Fuerza modo digital */
	CMD_MODE_ANALOG,            /**< Fuerza modo analógico */
	CMD_LOOPBACK_GET,           /**< Consulta el estado del loopback UART */
	CMD_LOOPBACK_ON,            /**< Habilita el loopback UART */
	CMD_LOOPBACK_OFF,           /**< Deshabilita el loopback UART */
} cmd_id_t;

/**
 * @brief   Inicializa el módulo parser de comandos, limpia buffers y FIFO.
 */
void cmdParser_init(void);

/**
 * @brief   FSM del parser. Debe llamarse periódicamente desde el bucle
 *          principal. Procesa un byte por invocación (no bloqueante).
 */
void cmdParser_poll(void);

/**
 * @brief   Imprime por UART la lista de comandos disponibles con su descripción.
 */
void cmdParser_printHelp(void);

/**
 * @brief   Extrae el próximo comando pendiente de la FIFO.
 *
 * @param   cmd     Puntero al cmd_id_t donde se escribe el comando.
 * @return  true si había un comando pendiente, false si la FIFO estaba vacía
 *          o @p cmd es NULL.
 */
bool_t cmd_getPendingCommand(cmd_id_t *cmd);

#endif /* API_INC_API_CMDPARSER_H_ */
