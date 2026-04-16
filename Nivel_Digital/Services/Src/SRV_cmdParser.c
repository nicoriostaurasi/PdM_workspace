/** @file  SRV_cmdParser.c
 *  @brief Parser de comandos por línea recibidos a través de UART.
 *
 *  Implementa una máquina de estados finitos (FSM) que recibe caracteres
 *  desde el módulo UART, arma líneas de comando, las tokeniza y busca
 *  coincidencia contra una tabla de comandos registrados. Los comandos
 *  reconocidos se encolan en una FIFO para ser consumidos por el módulo
 *  principal (digitalAngleMeter).
 *
 *  @date 3 abr 2026
 *  @author Ing. Nicolás Gabriel Rios Taurasi
 */

#include <string.h>
#include <ctype.h>

#include "SRV_cmdParser.h"
#include "BSP_uart.h"

/** @brief Estados posibles de la máquina de estados del parser de comandos */
typedef enum {
	CMD_IDLE = 0,     /**< Esperando el primer carácter de un nuevo comando */
	CMD_RECEIVING,    /**< Recibiendo caracteres de la línea de comando */
	CMD_PROCESS,      /**< Línea completa lista para ser procesada */
	CMD_ERROR,        /**< Ocurrió un error; se reporta y se vuelve a IDLE */
} cmdParserState_t;


/** @brief Detalle de un argumento simple asociado a un comando */
typedef struct{
	const char *argument;    /**< Argumento esperado (NULL si el comando no lleva argumento) */
	cmd_id_t id;             /**< Identificador del comando a encolar en la FIFO */
	const char *description; /**< Descripción del argumento para la ayuda */
} simpleArgumentDetail_t;

/** @brief Definición de un comando con sus argumentos simples */
typedef struct{
    const char *name;                       /**< Nombre del comando (en mayúsculas) */
    uint8_t availableDifferentArgs;         /**< Cantidad de variantes de argumento disponibles */
    simpleArgumentDetail_t *argumentDetail; /**< Puntero al arreglo de detalles de argumentos */
} commandWithSimpleArguments_t;

/** @brief Buffer para acumular la línea de comando recibida por UART */
static uint8_t uartRxBuffer[CMD_MAX_LINE];

/** @brief Estado actual de la FSM del parser de comandos */
static cmdParserState_t cmdParserStateFsm = CMD_IDLE;

/** @brief FIFO circular de comandos pendientes de ejecución */
static cmd_id_t cmdFifo[CMD_FIFO_SIZE];
static uint8_t cmdFifoHead = 0; /**< Índice de escritura de la FIFO de comandos */
static uint8_t cmdFifoTail = 0; /**< Índice de lectura de la FIFO de comandos */

/**
 * @brief Inserta un comando en la FIFO de comandos pendientes.
 *
 * @param id Identificador del comando a encolar
 * @return true si el comando se encoló exitosamente, false si la FIFO estaba llena
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

/**
 * @brief Extrae el próximo comando pendiente de la FIFO.
 *
 * @param cmd Puntero donde se almacena el identificador del comando extraído
 * @return true si se extrajo un comando, false si la FIFO estaba vacía o cmd es NULL
 */
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

/**
 * @brief Tokeniza la línea de comando recibida separando por espacios.
 *
 * Divide el string de entrada en tokens usando el espacio como delimitador
 * y los almacena en el arreglo argv.
 *
 * @param input Línea de comando a tokenizar (se modifica in-place por strtok)
 * @param argv Arreglo donde se almacenan los punteros a cada token
 * @return Cantidad de tokens encontrados, o -1 si se superó CMD_MAX_TOKENS
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

/**
 * @brief Envía por UART el mensaje de error correspondiente al tipo indicado.
 *
 * @param errorAction Tipo de error ocurrido durante el procesamiento del comando
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


/** @brief Argumentos disponibles para el comando HELP */
simpleArgumentDetail_t helpPayload[]={
		{NULL, CMD_HELP, "Print this help message"},
};

/** @brief Argumentos disponibles para el comando READ */
simpleArgumentDetail_t readPayload[]={
	{"ANGLE", CMD_READ_ANGLE, "Read the current angle from the sensor"},
	{"ACCELERATION", CMD_READ_ACCELERATION, "Read the current acceleration from the sensor"},
};

/** @brief Argumentos disponibles para el comando STATUS */
simpleArgumentDetail_t statusPayload[]={
		{NULL, CMD_STATUS, "Get the current status of the system"},
};

/** @brief Argumentos disponibles para el comando MODE */
simpleArgumentDetail_t modePayload[]={
		{NULL, CMD_MODE_GET, "Get the current display mode"},
		{"TOGGLE", CMD_MODE_TOGGLE, "Toggle between digital and analog mode"},
		{"DIGITAL", CMD_MODE_DIGITAL, "Set the display mode to digital"},
		{"ANALOG", CMD_MODE_ANALOG, "Set the display mode to analog"},
};

/** @brief Argumentos disponibles para el comando LOOPBACK */
simpleArgumentDetail_t loopbackPayload[]={
		{NULL, CMD_LOOPBACK_GET, "Get the current UART loopback state"},
		{"ON", CMD_LOOPBACK_ON, "Enable UART RX->TX loopback (echo)"},
		{"OFF", CMD_LOOPBACK_OFF, "Disable UART RX->TX loopback (echo)"},
};

/** @brief Tabla de comandos disponibles con sus argumentos asociados */
const commandWithSimpleArguments_t availableCommands[] = {
	{"HELP",1, helpPayload},
	{"READ",2, readPayload},
	{"STATUS",1, statusPayload},
	{"MODE",4, modePayload},
	{"LOOPBACK",3, loopbackPayload},
};

/**
 * @brief Procesa la línea de comando almacenada en el buffer y encola la acción.
 *
 * Tokeniza la línea, busca el comando en la tabla de comandos disponibles,
 * verifica los argumentos y encola el comando correspondiente en la FIFO.
 *
 * @return Estado del procesamiento (CMD_OK si fue exitoso, o el código de error)
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
 * @brief Inicializa el módulo parser de comandos.
 *
 * Reinicia la FSM al estado IDLE, limpia la FIFO de comandos y el buffer
 * de recepción.
 */
void cmdParser_init(void){
	cmdParserStateFsm = CMD_IDLE;
	cmdFifoHead = 0;
	cmdFifoTail = 0;
	memset(uartRxBuffer, 0, CMD_MAX_LINE);
}

/**
 * @brief Actualiza la FSM del parser de comandos (no bloqueante).
 *
 * Debe llamarse periódicamente desde el bucle principal. Procesa un byte
 * por invocación: en CMD_IDLE espera el primer carácter, en CMD_RECEIVING
 * acumula bytes hasta detectar fin de línea, en CMD_PROCESS tokeniza y
 * busca el comando, y en CMD_ERROR reporta el error por UART.
 */
void cmdParser_poll(void){
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
 * @brief Imprime por UART la lista de comandos disponibles con sus descripciones.
 *
 * Recorre la tabla de comandos registrados y envía el nombre de cada comando
 * junto con sus argumentos y descripciones.
 */
void cmdParser_printHelp(void){
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
