/*
 * API_cmdparser.c
 *
 *  Created on: 3 abr 2026
 *      Author: nicol
 */

#include "API_cmdparser.h"
#include "API_uart.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "stm32f4xx_hal.h"
#include "API_digital_angle_meter.h"

#define LD2_Pin GPIO_PIN_5
#define LD2_GPIO_Port GPIOA

/** @brief Tipo de función para manejar comandos */
typedef void (*command_handler_t)(uint8_t argc, char *argv[]);

/** @brief Tipos de estado para la máquina de estados del parser de comandos */
typedef enum {
	CMD_IDLE = 0,
	CMD_RECEIVING,
	CMD_PROCESS,
	CMD_ERROR,
	CMD_WAIT_FOR_EXCECUTION,
} cmdParserState_t;


/** @brief Estructura para definir un comando */
typedef struct{
	const char *argument;
	cmd_id_t id;
	const char *description;
} simpleArgumentDetail_t;

typedef struct{
    const char *name;
    uint8_t availableDifferentArgs;
    simpleArgumentDetail_t *argumentDetail;
} commandWithSimpleArguments_t;

static cmd_id_t currentCmdId;

/** @brief Buffer para recibir datos por UART */
static uint8_t uartRxBuffer[CMD_MAX_LINE];

static uint8_t digitalAngleMeterFsmBuffer[CMD_MAX_LINE];

/** @brief Máquina de estados del parser de comandos */
static cmdParserState_t cmdParserStateFsm = CMD_IDLE;

/** @brief Variables para almacenar los argumentos actuales y el handler del comando a ejecutar */
static char* currentArgv[CMD_MAX_TOKENS];

/** @brief Variable para almacenar el número de argumentos actuales */
static uint8_t currentArgc = 0;

/** @brief Variable para almacenar el handler del comando a ejecutar */
static command_handler_t currentCmdHandler;

static bool_t digitalAngleMeterCmd = false;

static cmd_event_t pendingCmd;


bool_t cmdGetPendingCommand(cmd_id_t *cmd)
{
    if (!pendingCmd.pending) {
        return false;
    }

    *cmd = pendingCmd.id;
    pendingCmd.pending = false;
    cmdParserStateFsm = CMD_IDLE;
    return true;
}

/** @brief Acción para el comando HELP */
void helpAction(void) {
	cmdPrintHelp();
}



/**** Accion para el comando STATUS ****/


/** @brief Función para tokenizar la línea de comando recibida
 *  @param input: La línea de comando a tokenizar
 *  @param argv: El array donde se almacenarán los tokens
 *  @return: El número de tokens encontrados, o -1 si hubo un error de overflow
 */
static int8_t tokenize(char *input, char *argv[])
{
    uint8_t argc = 0;
    char *token = strtok(input, " ");

    while (token != NULL) {
        if (argc >= CMD_MAX_TOKENS) {
            return -1;
        }

		argv[argc++] = token;
        token = strtok(NULL, " ");
    }
    return argc;
}

/** @brief Función para ejecutar la acción correspondiente a un error de comando
 *  @param errorAction: El tipo de error ocurrido
 */
static void cmdExecutError(cmd_status_t errorAction){
	switch(errorAction){
		case CMD_ERR_OVERFLOW: {
			uartSendString((uint8_t*)"\n\rERROR: line too long\r\n");
			break;
		}
		case CMD_ERR_SYNTAX: {
			uartSendString((uint8_t*)"\n\rERROR: Syntax error\r\n");
			break;
		}
		case CMD_ERR_UNKNOWN: {
			uartSendString((uint8_t*)"\n\rERROR: Unknown command\r\n");
			break;
		}
		case CMD_ERR_ARG: {
			uartSendString((uint8_t*)"\n\rERROR: Bad arguments\r\n");
			break;
		}
		default: {
			uartSendString((uint8_t*)"\n\rERROR: Unknown command\r\n");
			break;
		}
	}
}


simpleArgumentDetail_t helpPayload[]={
		{NULL, CMD_HELP, "Print this help message"},
};

simpleArgumentDetail_t readPayload[]={
	{"ANGLE", CMD_READ_ANGLE, "Read the current angle from the sensor"},
	{"ACCELERATION", CMD_READ_ACCELERATION, "Read the current acceleration from the sensor"},
};

simpleArgumentDetail_t statusPayload[]={
		{NULL, CMD_STATUS, "Get the current status of the system"},
};

simpleArgumentDetail_t modePayload[]={
		{NULL, CMD_MODE_GET, "Get the current display mode"},
		{"TOGGLE", CMD_MODE_TOGGLE, "Toggle between digital and analog mode"},
		{"DIGITAL", CMD_MODE_DIGITAL, "Set the display mode to digital"},
		{"ANALOG", CMD_MODE_ANALOG, "Set the display mode to analog"},
};

const commandWithSimpleArguments_t availableCommands[] = {
	{"HELP",1, helpPayload},
	{"READ",2, readPayload},
	{"STATUS",1, statusPayload},
	{"MODE",4, modePayload},
};

/** @brief Función para procesar la línea de comando recibida y ejecutar la acción correspondiente
 *  @return: El estado del procesamiento del comando
 */
static cmd_status_t cmdProcessLine(void)
{
	char* argv[CMD_MAX_TOKENS];
	char *expectedArg;
	int8_t argc = tokenize((char*)uartRxBuffer, argv);

	// Valido que los comandos sean los aceptados, si hay mas doy un overflow
    if (argc < 0) {
        return CMD_ERR_OVERFLOW;
    }

    // Valido que no sea una linea vacía
    if (argc == 0) {
        return CMD_ERR_SYNTAX;
    }

    // Recorro para checkear dinamicamente la lista de comandos disponibles
     for (uint8_t i = 0; i < (sizeof(availableCommands) / sizeof(availableCommands[0])); i++) {
		// Si el comando coincide, asigno el handler y los argumentos para su ejecución
		if (strcmp(argv[0], availableCommands[i].name) == 0) {
			for(uint8_t j=0; j<availableCommands[i].availableDifferentArgs;j++) {
	            expectedArg = availableCommands[i].argumentDetail[j].argument;
				if(expectedArg == NULL){
					if(argc==1){
						pendingCmd.id = availableCommands[i].argumentDetail[j].id;
						pendingCmd.pending = true;
						return CMD_OK;
					}
				}
				if (argc == 2 && strcmp(argv[1], expectedArg) == 0) {
					pendingCmd.id = availableCommands[i].argumentDetail[j].id;
				    pendingCmd.pending = true;
				    return CMD_OK;
				}
			}
			return CMD_ERR_ARG;
		}
    }
	return CMD_ERR_UNKNOWN;
}

/**
 * @brief Ejecuta la accion correspondiente
 */
static void cmdExecutAction(void){
	currentCmdHandler(currentArgc, currentArgv);
}

/**
 * @brief Inicializa el módulo parser de comandos
 */
void cmdParserInit(void){
	cmdParserStateFsm = CMD_IDLE;
	currentArgc = 0;
	memset(uartRxBuffer,0,CMD_MAX_LINE);
}

/**
 * @brief Máquina de estados del Paser. Debe ser llamada periódicamente desde el bucle
 * 		  Procesa hasta 16 bytes por invocación (no bloqueante).
 */
void cmdPoll(void){
	static uint8_t currentDataIndex = 0;
	static cmd_status_t cmdProcessStatus;
	static uint8_t currentRecChar = 0;
	switch(cmdParserStateFsm) {
		case CMD_IDLE:{
			if(uartRxPop(&currentRecChar)){
				// Si detecto un caracter válido de un nuevo comando, lo guardo y cambio de estado
				if (currentRecChar != '\r' && currentRecChar != '\n') {
					cmdParserStateFsm = CMD_RECEIVING;
					// limpio el buffer por si hay basura
					memset(uartRxBuffer,0,CMD_MAX_LINE);
					// Almaceno el primer caracter
					uartRxBuffer[0] = (char)toupper((unsigned char)currentRecChar);
					currentDataIndex = 1;
				}
			}
			break;
		}

		case CMD_RECEIVING:{
			if(uartRxPop(&currentRecChar)){
				if (currentRecChar == '\r' || currentRecChar == '\n'){
					// Una vez que se recibe el último caracter se procesa la trama
					cmdParserStateFsm = CMD_PROCESS;
					uartRxBuffer[currentDataIndex] = '\0';
					currentDataIndex++;
				}else{
					// Si hay caracteres válidos se continua almacenando
					uartRxBuffer[currentDataIndex] = (char)toupper((unsigned char)currentRecChar);
					currentDataIndex++;
				}
			}

			if(currentDataIndex>=CMD_MAX_LINE){
				cmdProcessStatus = CMD_ERR_OVERFLOW;
				cmdParserStateFsm = CMD_ERROR;
			}
			break;
		}

		case CMD_WAIT_FOR_EXCECUTION: {
			break;
		}

		case CMD_PROCESS:{
			cmdProcessStatus = cmdProcessLine();
			if(cmdProcessStatus == CMD_OK){
				cmdParserStateFsm = CMD_IDLE;
			} else {
				cmdParserStateFsm = CMD_ERROR;
			}
			break;
		}

		case CMD_ERROR:{
			cmdExecutError(cmdProcessStatus);
			cmdParserStateFsm = CMD_IDLE;
			break;
		}

		default:{
			cmdParserStateFsm = CMD_IDLE;
			break;
		}

	}
	return;
}

/**
 * @brief Imprime por UART la lista de comandos disponibles
 */
void cmdPrintHelp(void){
	uartSendString((uint8_t*)"\nAvailable Commands with description:\r\n");
 	for (uint8_t i = 0; i < (sizeof(availableCommands) / sizeof(availableCommands[0])); i++){
		uartSendString((uint8_t*)availableCommands[i].name);
		uartSendString((uint8_t*)"\r\n");
		for (uint8_t j = 0;j<availableCommands[i].availableDifferentArgs;j++){
			uartSendString((uint8_t*)"        ");
			uartSendString((uint8_t*)availableCommands[i].argumentDetail[j].argument);
			uartSendString((uint8_t*)" -> ");
			uartSendString((uint8_t*)availableCommands[i].argumentDetail[j].description);
			uartSendString((uint8_t*)"\r\n");
		}
	}
}


