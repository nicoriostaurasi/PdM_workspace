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

#define LD2_Pin GPIO_PIN_5
#define LD2_GPIO_Port GPIOA

typedef void (*command_handler_t)(uint8_t argc, char *argv[]);

typedef enum {
	CMD_IDLE = 0,
	CMD_RECEIVING,
	CMD_PROCESS,
	CMD_EXEC,
	CMD_ERROR,
} cmdParserState_t;

typedef struct{
    const char *name;
	uint8_t expectedArgs;
	command_handler_t handler;
} command2_t;

static uint8_t uartRxBuffer[CMD_MAX_LINE];
static cmdParserState_t cmdParserStateFsm = CMD_IDLE;
static char* currentArgv[CMD_MAX_TOKENS];
static uint8_t currentArgc = 0;
static command_handler_t currentCmdHandler;

static void helpAction(uint8_t argc, char *argv[])
{
	cmdPrintHelp();
}

static void ledAction(uint8_t argc, char *argv[])
{
    if (strcmp(argv[1], "ON") == 0) {
    	HAL_GPIO_WritePin(LD2_GPIO_Port,LD2_Pin,GPIO_PIN_SET);
		uartSendString((uint8_t*)"\nChanged LED status to ON!\r\n");
    } else if (strcmp(argv[1], "OFF") == 0) {
    	HAL_GPIO_WritePin(LD2_GPIO_Port,LD2_Pin,GPIO_PIN_RESET);
    	uartSendString((uint8_t*)"\nChanged LED status to OFF!\r\n");
    } else if (strcmp(argv[1], "TOGGLE") == 0) {
    	HAL_GPIO_TogglePin(LD2_GPIO_Port,LD2_Pin);
		uartSendString((uint8_t*)"\nToggled current LED STATUS!\r\n");
	} else {
		uartSendString((uint8_t*)"\nLED Command Bad Arguments...\r\n");
    }
}

static void statusAction(uint8_t argc, char *argv[]) {
	GPIO_PinState ret=HAL_GPIO_ReadPin(LD2_GPIO_Port,LD2_Pin);
	if(ret == GPIO_PIN_SET) {
		uartSendString((uint8_t*)"\nCurrent LED STATUS: ON\r\n");
	} else {
		uartSendString((uint8_t*)"\nCurrent LED STATUS: OFF\r\n");
	}
}

static void baudGetAction(uint8_t argc, char *argv[]) {
	uint32_t currentBaudrate = getCurrentBaudrate();
	char buffBaud[8];

	sprintf(buffBaud, "%lu", currentBaudrate);
	uartSendString((uint8_t*)"\nCurrent Baudrate: ");
	uartSendString((uint8_t*)buffBaud);
	uartSendString((uint8_t*)"\r\n");
}

static void baudSetAction(uint8_t argc, char *argv[]) {
	uint32_t newBaudrate = atoi(argv[1]);

	if(newBaudrate<=921600 && newBaudrate>=9600 ){
		uartSendString((uint8_t*)"\nChange current Baudrate to: ");
		uartSendString((uint8_t*)argv[1]);
		uartSendString((uint8_t*)"\r\n");
		changeCurrentBaudrate(newBaudrate);
	} else {
		uartSendString((uint8_t*)"\nERROR: invalid baudrate to change\r\n");
	}

}

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

const command2_t commandTable2[] = {
	{"HELP",1, helpAction},
	{"LED",2, ledAction},
	{"STATUS",1, statusAction},
	{"BAUD?",1, baudGetAction},
	{"BAUD=",2, baudSetAction},
};

static cmd_status_t cmdProcessLine(void)
{
	char* argv[CMD_MAX_TOKENS];
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
    for (uint8_t i = 0; i < (sizeof(commandTable2) / sizeof(commandTable2[0])); i++) {
		if (strcmp(argv[0], commandTable2[i].name) == 0) {
			if(commandTable2[i].expectedArgs == argc){
				currentArgc = argc;
				memcpy(&currentArgv, &argv, sizeof(char*) * argc);
				currentCmdHandler = commandTable2[i].handler;
				return CMD_OK;
			} else{
				return CMD_ERR_ARG;
			}
		}
    }

    // no existe comando
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
	uint8_t aux;
	switch(cmdParserStateFsm) {
		case CMD_IDLE:{
			uartReceiveStringSize(&aux, 1);
			if(isNewDataOnRx()){
				uartSendStringSize(&aux,1);
				// Si detecto un caracter válido de un nuevo comando, lo guardo y cambio de estado
				if (aux != '\r' && aux != '\n') {
					cmdParserStateFsm = CMD_RECEIVING;
					// limpio el buffer por si hay basura
					memset(uartRxBuffer,0,CMD_MAX_LINE);
					// Almaceno el primer caracter
					uartRxBuffer[0] = (char)toupper((unsigned char)aux);
					currentDataIndex = 1;
				}
			}
			break;
		}

		case CMD_RECEIVING:{
			uartReceiveStringSize(&aux, 1);
			if(isNewDataOnRx()){
				uartSendStringSize(&aux,1);
				if (aux == '\r' || aux == '\n'){
					// Una vez que se recibe el último caracter se procesa la trama
					cmdParserStateFsm = CMD_PROCESS;
					uartRxBuffer[currentDataIndex] = '\0';
					currentDataIndex++;
				}else{
					// Si hay caracteres válidos se continua almacenando
					uartRxBuffer[currentDataIndex] = (char)toupper((unsigned char)aux);
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
				cmdParserStateFsm = CMD_EXEC;
			} else {
				cmdParserStateFsm = CMD_ERROR;
			}
			break;
		}

		case CMD_EXEC:{
			cmdExecutAction();
			cmdParserStateFsm = CMD_IDLE;
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
	uartSendString((uint8_t*)"\nAvailable Commands:\r\n");
	for (uint8_t i = 0; i < (sizeof(commandTable2) / sizeof(commandTable2[0])); i++){
		uartSendString((uint8_t*)commandTable2[i].name);
		uartSendString((uint8_t*)"\r\n");
	}
}


