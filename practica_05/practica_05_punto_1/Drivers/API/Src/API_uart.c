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

/** Estructura para la instancia de UART */
static UART_HandleTypeDef apiUartInstance;

/** Variable para indicar si el módulo está inicializado */
static bool_t isModuleInit = false;

/** @brief Función para imprimir mensajes de inicialización de UART
  * @param pstring: Puntero al string a imprimir
  * @param bufferSize: Tamaño del buffer para limpiar después de imprimir
  */
 static void uartInitPrint(uint8_t * pstring, size_t bufferSize){
	uartSendString(pstring);
	memset(pstring,0,bufferSize);
}

/** @brief Función para inicializar la UART
  * @return: true si la inicialización fue exitosa, false en caso contrario
  */
bool_t uartInit(){
	uint8_t buffConfig[BUFFER_LENGTH];

	// Configuración de la instancia de UART
	apiUartInstance.Instance = USART2;
	apiUartInstance.Init.BaudRate = 115200;
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
		while(1) {
			// Intentar recibir el string por UART, reintentando hasta el número máximo de intentos, esquema bloqueante
			ret = HAL_UART_Receive(&apiUartInstance, pstring, size, UART_TRANSMIT_TIMEOUT);
			if(ret == HAL_OK) {
				return;
			}
		}
	}
}
