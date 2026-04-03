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

#define BUFFER_LENGTH 64

static UART_HandleTypeDef apiUartInstance;
static bool_t isModuleInit = false;


static void uartInitPrint(uint8_t * pstring, size_t bufferSize){
	uartSendString(pstring);
	memset(pstring,0,bufferSize);
}

bool_t uartInit(){
	uint8_t buffConfig[BUFFER_LENGTH];

	apiUartInstance.Instance = USART2;
	apiUartInstance.Init.BaudRate = 115200;
	apiUartInstance.Init.WordLength = UART_WORDLENGTH_8B;
	apiUartInstance.Init.StopBits = UART_STOPBITS_1;
	apiUartInstance.Init.Parity = UART_PARITY_NONE;
	apiUartInstance.Init.Mode = UART_MODE_TX_RX;
	apiUartInstance.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	apiUartInstance.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&apiUartInstance) != HAL_OK)
	{
		return false;
	} else {
		isModuleInit = true;

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

void uartSendString(uint8_t * pstring){
	uint16_t stringCharCounter = 0;
	uint16_t attemptCounter = 0;
	uint16_t stringLengthToSend = 0;

	if(!isModuleInit) {
		return;
	}

	if(pstring==NULL){
		return;
	}

	for(stringCharCounter=0;stringCharCounter<=UART_MAX_STRING_LENGTH;stringCharCounter++) {
		if(pstring[stringCharCounter]== '\0') {
			stringLengthToSend = stringCharCounter;
			if(stringLengthToSend<=UART_MIN_STRING_LENGTH) {
				return;
			}

			for(attemptCounter=0;attemptCounter<UART_MAX_TRANSMIT_ATTEMPTS;attemptCounter++) {
				if(HAL_UART_Transmit(&apiUartInstance, pstring, stringLengthToSend, UART_TRANSMIT_TIMEOUT)==HAL_OK) {
					return;
				}
			}
			return;
		}
	}
}

void uartSendStringSize(uint8_t * pstring, uint16_t size){
	uint16_t attemptCounter = 0;

	if(!isModuleInit) {
		return;
	}

	if(pstring == NULL) {
		return;
	}

	if(UART_MIN_STRING_LENGTH <= size && size <= UART_MAX_STRING_LENGTH) {
		for(attemptCounter=0;attemptCounter<UART_MAX_TRANSMIT_ATTEMPTS;attemptCounter++) {
			if(HAL_UART_Transmit(&apiUartInstance, pstring, size, UART_TRANSMIT_TIMEOUT)==HAL_OK) {
				return;
			}
		}
		return;
	}
}

void uartReceiveStringSize(uint8_t * pstring, uint16_t size){
	HAL_StatusTypeDef ret;

	if(!isModuleInit) {
		return;
	}

	if(pstring == NULL) {
		return;
	}

	if(UART_MIN_STRING_LENGTH <= size && size <= UART_MAX_STRING_LENGTH) {
		while(1) {
			ret = HAL_UART_Receive(&apiUartInstance, pstring, size, UART_TRANSMIT_TIMEOUT);
			if(ret == HAL_OK) {
				return;
			}
		}
	}
}
