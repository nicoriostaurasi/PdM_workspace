/*
 * API_cmdparser.h
 *
 *  Created on: 3 abr 2026
 *      Author: nicol
 */

#ifndef API_INC_API_CMDPARSER_H_
#define API_INC_API_CMDPARSER_H_

#include "API_delay.h"

//#include <stdbool.h>
/* Definiciones de constantes para el parser de comandos */
#define CMD_MAX_LINE 64
#define CMD_MAX_TOKENS 3
#define CMD_FIFO_SIZE 4

/** @brief Tipos de estado para la máquina de estados del parser de comandos */
typedef enum {
	CMD_OK = 0,
	CMD_ERR_OVERFLOW,
	CMD_ERR_SYNTAX,
	CMD_ERR_UNKNOWN,
	CMD_ERR_ARG,
} cmd_status_t;

typedef enum{
	CMD_HELP = 0,
	CMD_READ_ANGLE,
	CMD_READ_ACCELERATION,
	CMD_STATUS,
	CMD_MODE_GET,
	CMD_MODE_TOGGLE,
	CMD_MODE_DIGITAL,
	CMD_MODE_ANALOG,
	CMD_LOOPBACK_GET,
	CMD_LOOPBACK_ON,
	CMD_LOOPBACK_OFF,
} cmd_id_t;

/**
 * @brief Inicializa el módulo parser de comandos
 */
void cmdParserInit(void);

/**
 * @brief Máquina de estados del Paser. Debe ser llamada periódicamente desde el bucle
 * 		  Procesa hasta 16 bytes por invocación (no bloqueante).
 */
void cmdPoll(void);

/**
 * @brief Imprime por UART la lista de comandos disponibles
 */
void cmdPrintHelp(void);

bool_t cmdGetPendingCommand(cmd_id_t *cmd);

#endif /* API_INC_API_CMDPARSER_H_ */
