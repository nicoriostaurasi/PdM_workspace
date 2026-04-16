/*
 * API_cmdparser.c
 *
 *  Created on: 3 abr 2026
 *      Author: nicol
 */

#include <string.h>
#include <ctype.h>

#include "API_cmdparser.h"
#include "API_uart.h"

/** @brief Tipos de estado para la máquina de estados del parser de comandos */
typedef enum {
	CMD_IDLE = 0,
	CMD_RECEIVING,
	CMD_PROCESS,
	CMD_ERROR,
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

/** @brief Buffer para recibir datos por UART */
static uint8_t uartRxBuffer[CMD_MAX_LINE];

/** @brief Máquina de estados del parser de comandos */
static cmdParserState_t cmdParserStateFsm = CMD_IDLE;

/** @brief FIFO circular de comandos pendientes de ejecución */
static cmd_id_t cmdFifo[CMD_FIFO_SIZE];
static uint8_t cmdFifoHead = 0;
static uint8_t cmdFifoTail = 0;

/**
 * @brief Empuja un comando en la FIFO.
 * @return true si entró, false si la FIFO estaba llena.
 */
static bool_t cmdFifoPush(cmd_id_t id)
{
    uint8_t nextHead = (cmdFifoHead + 1U) % CMD_FIFO_SIZE;
    if (nextHead == cmdFifoTail) {
        return false; // llena, descartamos el comando más reciente
    }
    cmdFifo[cmdFifoHead] = id;
    cmdFifoHead = nextHead;
    return true;
}

bool_t cmd_getPendingCommand(cmd_id_t *cmd)
{
    if (cmd == NULL) {
        return false;
    }
    if (cmdFifoHead == cmdFifoTail) {
        return false; // vacía
    }

    *cmd = cmdFifo[cmdFifoTail];
    cmdFifoTail = (cmdFifoTail + 1U) % CMD_FIFO_SIZE;
    return true;
}

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
			uart_sendString((uint8_t*)"\n\rERROR: line too long\r\n");
			break;
		}
		case CMD_ERR_SYNTAX: {
			uart_sendString((uint8_t*)"\n\rERROR: Syntax error\r\n");
			break;
		}
		case CMD_ERR_UNKNOWN: {
			uart_sendString((uint8_t*)"\n\rERROR: Unknown command\r\n");
			break;
		}
		case CMD_ERR_ARG: {
			uart_sendString((uint8_t*)"\n\rERROR: Bad arguments\r\n");
			break;
		}
		default: {
			uart_sendString((uint8_t*)"\n\rERROR: Unknown command\r\n");
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

simpleArgumentDetail_t loopbackPayload[]={
		{NULL, CMD_LOOPBACK_GET, "Get the current UART loopback state"},
		{"ON", CMD_LOOPBACK_ON, "Enable UART RX->TX loopback (echo)"},
		{"OFF", CMD_LOOPBACK_OFF, "Disable UART RX->TX loopback (echo)"},
};

const commandWithSimpleArguments_t availableCommands[] = {
	{"HELP",1, helpPayload},
	{"READ",2, readPayload},
	{"STATUS",1, statusPayload},
	{"MODE",4, modePayload},
	{"LOOPBACK",3, loopbackPayload},
};

/** @brief Función para procesar la línea de comando recibida y ejecutar la acción correspondiente
 *  @return: El estado del procesamiento del comando
 */
static cmd_status_t cmdProcessLine(void)
{
	char* argv[CMD_MAX_TOKENS];
	const char *expectedArg;
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
						if(!cmdFifoPush(availableCommands[i].argumentDetail[j].id)){
							return CMD_ERR_OVERFLOW;
						}
						return CMD_OK;
					}
				}
				if (argc == 2 && strcmp(argv[1], expectedArg) == 0) {
					if(!cmdFifoPush(availableCommands[i].argumentDetail[j].id)){
						return CMD_ERR_OVERFLOW;
					}
					return CMD_OK;
				}
			}
			return CMD_ERR_ARG;
		}
    }
	return CMD_ERR_UNKNOWN;
}

/**
 * @brief Inicializa el módulo parser de comandos
 */
void cmd_parserInit(void){
	cmdParserStateFsm = CMD_IDLE;
	cmdFifoHead = 0;
	cmdFifoTail = 0;
	memset(uartRxBuffer, 0, CMD_MAX_LINE);
}

/**
 * @brief FSM del parser. Debe llamarse periódicamente desde el bucle principal.
 *        Procesa un byte por invocación (no bloqueante).
 */
void cmd_poll(void){
	static uint8_t currentDataIndex = 0;
	static cmd_status_t cmdProcessStatus;
	static uint8_t currentRecChar = 0;
	switch(cmdParserStateFsm) {
		case CMD_IDLE:{
			if(uart_rxPop(&currentRecChar)){
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
			if(uart_rxPop(&currentRecChar)){
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
void cmd_printHelp(void){
	uart_sendString((uint8_t*)"\nAvailable Commands with description:\r\n");
 	for (uint8_t i = 0; i < (sizeof(availableCommands) / sizeof(availableCommands[0])); i++){
		uart_sendString((uint8_t*)availableCommands[i].name);
		uart_sendString((uint8_t*)"\r\n");
		for (uint8_t j = 0;j<availableCommands[i].availableDifferentArgs;j++){
			uart_sendString((uint8_t*)"        ");
			uart_sendString((uint8_t*)availableCommands[i].argumentDetail[j].argument);
			uart_sendString((uint8_t*)" -> ");
			uart_sendString((uint8_t*)availableCommands[i].argumentDetail[j].description);
			uart_sendString((uint8_t*)"\r\n");
		}
	}
}


