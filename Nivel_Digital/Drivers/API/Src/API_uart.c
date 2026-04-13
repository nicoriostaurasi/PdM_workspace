/*
 * API_uart.c
 *
 *  Created on: 2 abr 2026
 *      Author: nicol
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stm32f4xx_hal.h"
#include "API_uart.h"

/* Buffer Lenght para la configuración */
#define BUFFER_LENGTH 64

#define UART_RX_FIFO_SIZE 64

static volatile uint8_t rxFifo[UART_RX_FIFO_SIZE];
static volatile uint16_t rxHead = 0;
static volatile uint16_t rxTail = 0;

#define UART_TX_FIFO_SIZE 64

static volatile uint8_t txFifo[UART_TX_FIFO_SIZE];
static volatile uint16_t txHead = 0;
static volatile uint16_t txTail = 0;
static volatile bool_t txBusy = false;
static uint8_t txCurrentByte;

/** Estructura para la instancia de UART */
static UART_HandleTypeDef apiUartInstance;

/** Variable para el baudrate de la UART*/
static uint32_t currentUartBaudrate;

/** Variable para indicar si el módulo está inicializado */
static bool_t isModuleInit = false;

/** Variable para saber si hubo nuevos datos en RX */
static bool_t isNewData = false;

/** Variable para habilitar el loopback entre RX y TX*/
static bool_t loopbackEnable = true;

/** Variable para almacenar los caracteres recibidos*/
static uint8_t charRx;


/** @brief Función para imprimir mensajes de inicialización de UART
  * @param pstring: Puntero al string a imprimir
  * @param bufferSize: Tamaño del buffer para limpiar después de imprimir
  */
static void uartInitPrint(uint8_t * pstring, size_t bufferSize){
	uartSendString(pstring);
	memset(pstring,0,bufferSize);
}

void USART2_IRQHandler(void)
{
	HAL_UART_IRQHandler(&apiUartInstance);
}


static bool_t uartTxPop(uint8_t *data)
{
    if (txHead == txTail) {
        return false;
    }

    *data = txFifo[txTail];
    txTail = (txTail + 1) % UART_TX_FIFO_SIZE;
    return true;
}

static void uartStartTxIT(void)
{
    if (txBusy) {
        return;
    }

    if (uartTxPop(&txCurrentByte)) {
        txBusy = true;
        HAL_UART_Transmit_IT(&apiUartInstance, &txCurrentByte, 1);
    }
}

static void uartRxPush(uint8_t data)
{
    uint16_t nextHead = (rxHead + 1) % UART_RX_FIFO_SIZE;

    if (nextHead == rxTail) {
        return;
    }

    rxFifo[rxHead] = data;
    rxHead = nextHead;
}

bool_t uartRxPop(uint8_t *data)
{
    if (rxHead == rxTail) {
        return false;
    }

    *data = rxFifo[rxTail];
    rxTail = (rxTail + 1) % UART_RX_FIFO_SIZE;

    return true;
}

static bool_t uartTxPush(uint8_t data)
{
    uint16_t nextHead = (txHead + 1) % UART_TX_FIFO_SIZE;

    if (nextHead == txTail) {
        return false; // lleno
    }

    txFifo[txHead] = data;
    txHead = nextHead;
    return true;
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance != USART2) {
        return;
    }

    if(loopbackEnable) {
    	 uartTxPush(charRx);
    	 uartStartTxIT();
    }

    uartRxPush(charRx);
    HAL_UART_Receive_IT(&apiUartInstance, &charRx, 1);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance != USART2) {
        return;
    }

    if (uartTxPop(&txCurrentByte)) {
        HAL_UART_Transmit_IT(&apiUartInstance, &txCurrentByte, 1);
    } else {
        txBusy = false;
    }
}

/** @brief Función para obtener la tasa de baudios actual
 *  @return: La tasa de baudios actual
 */
uint32_t getCurrentBaudrate(void){
	return currentUartBaudrate;
}

/** @brief Función para cambiar la tasa de baudios
 *  @param newBaudrate: La nueva tasa de baudios a configurar
 *  @return: true si el cambio fue exitoso, false en caso contrario
 */
bool_t changeCurrentBaudrate(uint32_t newBaudrate){
	uint8_t buffConfig[BUFFER_LENGTH];
	isModuleInit = false;

	// Configuración de la instancia de UART
	apiUartInstance.Instance = USART2;
	apiUartInstance.Init.BaudRate = newBaudrate;
	apiUartInstance.Init.WordLength = UART_WORDLENGTH_8B;
	apiUartInstance.Init.StopBits = UART_STOPBITS_1;
	apiUartInstance.Init.Parity = UART_PARITY_NONE;
	apiUartInstance.Init.Mode = UART_MODE_TX_RX;
	apiUartInstance.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	apiUartInstance.Init.OverSampling = UART_OVERSAMPLING_16;
	// Inicialización de la UART
	if (HAL_UART_Init(&apiUartInstance) != HAL_OK)
	{
		return false;
	} else {
		currentUartBaudrate = newBaudrate;
		isModuleInit = true;
		// Imprimir configuración de UART
		sprintf((char*)buffConfig,"Uart Configurada!\r\n");
		uartInitPrint(buffConfig, BUFFER_LENGTH);

		sprintf((char*)buffConfig,"BaudRate: %ld \r\n",apiUartInstance.Init.BaudRate);
		uartInitPrint(buffConfig, BUFFER_LENGTH);

		sprintf((char*)buffConfig,"Longitud de Palabra: 8 BITS\r\n");
		uartInitPrint(buffConfig, BUFFER_LENGTH);

		sprintf((char*)buffConfig,"Bit de Paridad: NO\r\n");
		uartInitPrint(buffConfig, BUFFER_LENGTH);

		sprintf((char*)buffConfig,"Bits de Stop: 1 BIT\r\n");
		uartInitPrint(buffConfig, BUFFER_LENGTH);

		return true;
	}
}

/** @brief Función para saber si hubo una lectura exitosa
 *  @return: true si hay nuevos datos, false en caso contrario
 */
bool isNewDataOnRx(void) {
	if(isNewData){
		isNewData=false;
		return true;
	} else{
		return false;
	}
}

/** @brief Función para inicializar la UART
  * @return: true si la inicialización fue exitosa, false en caso contrario
  */
bool_t uartInit(){
	uint8_t buffConfig[BUFFER_LENGTH];

	currentUartBaudrate = 115200;
	// Configuración de la instancia de UART
	apiUartInstance.Instance = USART2;
	apiUartInstance.Init.BaudRate = 115200;
	apiUartInstance.Init.WordLength = UART_WORDLENGTH_8B;
	apiUartInstance.Init.StopBits = UART_STOPBITS_1;
	apiUartInstance.Init.Parity = UART_PARITY_NONE;
	apiUartInstance.Init.Mode = UART_MODE_TX_RX;
	apiUartInstance.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	apiUartInstance.Init.OverSampling = UART_OVERSAMPLING_16;
	HAL_NVIC_EnableIRQ(USART2_IRQn);
	HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
	// Inicialización de la UART
	if (HAL_UART_Init(&apiUartInstance) != HAL_OK ||
		HAL_UART_Receive_IT(&apiUartInstance,&charRx,1) != HAL_OK){
		return false;
	} else {
		isModuleInit = true;
		// Imprimir configuración de UART
		sprintf((char*)buffConfig,"Uart Configurada!\r\n");
		uartInitPrint(buffConfig, BUFFER_LENGTH);

		sprintf((char*)buffConfig,"BaudRate: %ld \r\n",apiUartInstance.Init.BaudRate);
		uartInitPrint(buffConfig, BUFFER_LENGTH);

		sprintf((char*)buffConfig,"Longitud de Palabra: 8 BITS\r\n");
		uartInitPrint(buffConfig, BUFFER_LENGTH);

		sprintf((char*)buffConfig,"Bit de Paridad: NO\r\n");
		uartInitPrint(buffConfig, BUFFER_LENGTH);

		sprintf((char*)buffConfig,"Bits de Stop: 1 BIT\r\n");
		uartInitPrint(buffConfig, BUFFER_LENGTH);

		return true;
	}
}

/** @brief Función para enviar una cadena de caracteres por UART
  * @param pstring: Puntero al string a enviar
  */
void uartSendString(uint8_t * pstring){
	uint16_t stringCharCounter = 0;
	uint16_t attemptCounter = 0;
	uint16_t stringLengthToSend = 0;

	// Validar que el módulo esté inicializado y que el puntero no sea NULL
	if(!isModuleInit) {
		return;
	}

	// Validar que el puntero no sea NULL
	if(pstring==NULL){
		return;
	}

	// Contar la longitud del string hasta el carácter nulo
	for(stringCharCounter=0;stringCharCounter<=UART_MAX_STRING_LENGTH;stringCharCounter++) {
		if(pstring[stringCharCounter]== '\0') {
			stringLengthToSend = stringCharCounter;
			if(stringLengthToSend<=UART_MIN_STRING_LENGTH) {
				return;
			}
			// Intentar enviar el string por UART, reintentando hasta el número máximo de intentos
			for(attemptCounter=0;attemptCounter<UART_MAX_TRANSMIT_ATTEMPTS;attemptCounter++) {
				if(HAL_UART_Transmit(&apiUartInstance, pstring, stringLengthToSend, UART_TRANSMIT_TIMEOUT)==HAL_OK) {
					return;
				}
			}
			return;
		}
	}
}

/** @brief Función para enviar una cadena de caracteres por UART con un tamaño específico
  * @param pstring: Puntero al string a enviar
  * @param size: Tamaño del string a enviar
  */
void uartSendStringSize(uint8_t * pstring, uint16_t size){
	uint16_t attemptCounter = 0;

	// Validar que el módulo esté inicializado y que el puntero no sea NULL
	if(!isModuleInit) {
		return;
	}

	// Validar que el puntero no sea NULL
	if(pstring == NULL) {
		return;
	}

	// Validar que el tamaño esté dentro de los límites permitidos
	if(UART_MIN_STRING_LENGTH <= size && size <= UART_MAX_STRING_LENGTH) {
		for(attemptCounter=0;attemptCounter<UART_MAX_TRANSMIT_ATTEMPTS;attemptCounter++) {
			// Intentar enviar el string por UART, reintentando hasta el número máximo de intentos
			if(HAL_UART_Transmit(&apiUartInstance, pstring, size, UART_TRANSMIT_TIMEOUT)==HAL_OK) {
				return;
			}
		}
		return;
	}
}

/** @brief Función para recibir una cadena de caracteres por UART con un tamaño específico
  * @param pstring: Puntero al buffer donde se almacenará la cadena recibida
  * @param size: Tamaño del string a recibir
  */
void uartReceiveStringSize(uint8_t * pstring, uint16_t size){
	HAL_StatusTypeDef ret;

	// Validar que el módulo esté inicializado y que el puntero no sea NULL
	if(!isModuleInit) {
		return;
	}

	// Validar que el puntero no sea NULL
	if(pstring == NULL) {
		return;
	}

	// Validar que el tamaño esté dentro de los límites permitidos
	if(UART_MIN_STRING_LENGTH <= size && size <= UART_MAX_STRING_LENGTH) {
		// Intentar recibir el string por UART, reintentando hasta el número máximo de intentos, esquema bloqueante
		ret = HAL_UART_Receive(&apiUartInstance, pstring, size, UART_TRANSMIT_TIMEOUT);
		if(ret == HAL_OK) {
			isNewData=true;
			return;
		}
	}
}
