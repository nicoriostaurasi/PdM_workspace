/** @file  BSP_uart.c
 *  @brief Módulo de comunicación UART por interrupciones con buffers FIFO circulares.
 *
 *  Implementa la inicialización, transmisión y recepción de datos a través de
 *  USART2 utilizando interrupciones (HAL_IT). Incluye soporte para loopback
 *  (eco) de RX hacia TX y gestión de buffers FIFO tanto para TX como para RX.
 *
 *  @date 2 abr 2026
 *  @author Ing. Nicolás Gabriel Rios Taurasi
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "BSP_uart.h"
#include "stm32f4xx_hal.h"

/** @brief Longitud del buffer auxiliar para mensajes de configuración */
#define BUFFER_LENGTH 64

/** @brief Tamaño del buffer FIFO circular de recepción (en bytes) */
#define UART_RX_FIFO_SIZE 1024

static volatile uint8_t rxFifo[UART_RX_FIFO_SIZE]; /**< Buffer FIFO circular de recepción */
static volatile uint16_t rxHead = 0;               /**< Índice de escritura del FIFO RX */
static volatile uint16_t rxTail = 0;               /**< Índice de lectura del FIFO RX */

/** @brief Tamaño del buffer FIFO circular de transmisión (en bytes) */
#define UART_TX_FIFO_SIZE 1024

static volatile uint8_t txFifo[UART_TX_FIFO_SIZE]; /**< Buffer FIFO circular de transmisión */
static volatile uint16_t txHead = 0;               /**< Índice de escritura del FIFO TX */
static volatile uint16_t txTail = 0;               /**< Índice de lectura del FIFO TX */
static volatile bool_t txBusy = false;              /**< Indica si hay una transmisión IT en curso */
static uint8_t txCurrentByte;                       /**< Byte actualmente en transmisión */

/** @brief Estructura para la instancia de UART (USART2) */
static UART_HandleTypeDef apiUartInstance;

/** @brief Tasa de baudios actual configurada en la UART */
static uint32_t currentUartBaudrate;

/** @brief Indica si el módulo UART fue inicializado correctamente */
static bool_t isModuleInit = false;

/** @brief Indica si se recibieron nuevos datos por RX (modo bloqueante) */
static bool_t isNewData = false;

/** @brief Habilita o deshabilita el loopback (eco) de RX hacia TX */
static volatile bool_t loopbackEnable = true;

/** @brief Último carácter recibido por interrupción */
static uint8_t charRx;


/**
 * @brief Imprime un mensaje por UART y limpia el buffer utilizado.
 *
 * Función auxiliar usada durante la inicialización para enviar mensajes
 * de configuración y luego limpiar el buffer temporal.
 *
 * @param pstring Puntero al string a imprimir
 * @param bufferSize Tamaño del buffer a limpiar después de imprimir
 */
static void uart_initPrint(uint8_t * pstring, size_t bufferSize){
	uart_sendString(pstring);
	memset(pstring,0,bufferSize);
}

/**
 * @brief Manejador de la interrupción de USART2.
 *
 * Redirige la interrupción al handler genérico de HAL para UART.
 */
void USART2_IRQHandler(void)
{
	HAL_UART_IRQHandler(&apiUartInstance);
}


/**
 * @brief Extrae un byte del buffer FIFO de transmisión.
 *
 * @param data Puntero donde se almacena el byte extraído
 * @return true si se extrajo un byte exitosamente, false si el FIFO estaba vacío
 */
static bool_t uart_txPop(uint8_t *data)
{
    if (txHead == txTail) {
        return false;
    }

    *data = txFifo[txTail];
    txTail = (txTail + 1) % UART_TX_FIFO_SIZE;
    return true;
}

/**
 * @brief Inicia la transmisión por interrupción si no hay una en curso.
 *
 * Extrae el próximo byte del FIFO TX e invoca HAL_UART_Transmit_IT.
 * Si ya hay una transmisión activa (txBusy), no realiza ninguna acción.
 */
static void uart_startTxIT(void)
{
    if (txBusy) {
        return;
    }

    if (uart_txPop(&txCurrentByte)) {
        txBusy = true;
        HAL_UART_Transmit_IT(&apiUartInstance, &txCurrentByte, 1);
    }
}

/**
 * @brief Inserta un byte en el buffer FIFO de recepción.
 *
 * Si el FIFO está lleno, el dato se descarta silenciosamente.
 *
 * @param data Byte a insertar en el FIFO RX
 */
static void uart_rxPush(uint8_t data)
{
    uint16_t nextHead = (rxHead + 1) % UART_RX_FIFO_SIZE;

    if (nextHead == rxTail) {
        return;
    }

    rxFifo[rxHead] = data;
    rxHead = nextHead;
}

/**
 * @brief Extrae un byte del buffer FIFO de recepción.
 *
 * @param data Puntero donde se almacena el byte extraído
 * @return true si se extrajo un byte exitosamente, false si el FIFO estaba vacío
 */
bool_t uart_rxPop(uint8_t *data)
{
    if (rxHead == rxTail) {
        return false;
    }

    *data = rxFifo[rxTail];
    rxTail = (rxTail + 1) % UART_RX_FIFO_SIZE;

    return true;
}

/**
 * @brief Inserta un byte en el buffer FIFO de transmisión.
 *
 * @param data Byte a insertar en el FIFO TX
 * @return true si se insertó correctamente, false si el FIFO estaba lleno
 */
static bool_t uart_txPush(uint8_t data)
{
    uint16_t nextHead = (txHead + 1) % UART_TX_FIFO_SIZE;

    if (nextHead == txTail) {
        return false; // lleno
    }

    txFifo[txHead] = data;
    txHead = nextHead;
    return true;
}


/**
 * @brief Callback de HAL invocado al completarse la recepción de un byte.
 *
 * Si el loopback está habilitado, reenvía el byte recibido por TX.
 * Almacena el byte en el FIFO RX y vuelve a habilitar la recepción IT.
 *
 * @param huart Puntero al handle de UART que generó la interrupción
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance != USART2) {
        return;
    }

    if(loopbackEnable) {
    	 uart_txPush(charRx);
    	 uart_startTxIT();
    }

    uart_rxPush(charRx);
    HAL_UART_Receive_IT(&apiUartInstance, &charRx, 1);
}

/**
 * @brief Callback de HAL invocado al completarse la transmisión de un byte.
 *
 * Extrae el siguiente byte del FIFO TX y continúa la transmisión.
 * Si el FIFO está vacío, marca txBusy como false.
 *
 * @param huart Puntero al handle de UART que generó la interrupción
 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance != USART2) {
        return;
    }

    if (uart_txPop(&txCurrentByte)) {
        HAL_UART_Transmit_IT(&apiUartInstance, &txCurrentByte, 1);
    } else {
        txBusy = false;
    }
}

/**
 * @brief Obtiene la tasa de baudios actual configurada en la UART.
 *
 * @return Tasa de baudios actual (por ejemplo, 115200)
 */
uint32_t uart_getCurrentBaudrate(void){
	return currentUartBaudrate;
}

/**
 * @brief Cambia la tasa de baudios de la UART y reinicializa el periférico.
 *
 * Reconfigura USART2 con el nuevo baudrate e imprime la configuración
 * resultante por la misma UART.
 *
 * @param newBaudrate Nueva tasa de baudios a configurar
 * @return true si el cambio fue exitoso, false si HAL_UART_Init falló
 */
bool_t uart_changeCurrentBaudrate(uint32_t newBaudrate){
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
		uart_initPrint(buffConfig, BUFFER_LENGTH);

		sprintf((char*)buffConfig,"BaudRate: %ld \r\n",apiUartInstance.Init.BaudRate);
		uart_initPrint(buffConfig, BUFFER_LENGTH);

		sprintf((char*)buffConfig,"Longitud de Palabra: 8 BITS\r\n");
		uart_initPrint(buffConfig, BUFFER_LENGTH);

		sprintf((char*)buffConfig,"Bit de Paridad: NO\r\n");
		uart_initPrint(buffConfig, BUFFER_LENGTH);

		sprintf((char*)buffConfig,"Bits de Stop: 1 BIT\r\n");
		uart_initPrint(buffConfig, BUFFER_LENGTH);

		return true;
	}
}

/**
 * @brief Indica si hubo nuevos datos recibidos por RX (modo bloqueante).
 *
 * Consulta y consume el flag de nueva data. Una vez leído, el flag se
 * resetea automáticamente.
 *
 * @return true si hay nuevos datos disponibles, false en caso contrario
 */
bool uart_isNewDataOnRx(void) {
	if(isNewData){
		isNewData=false;
		return true;
	} else{
		return false;
	}
}

/**
 * @brief Inicializa la UART (USART2) a 115200 bps con interrupciones.
 *
 * Configura USART2 en modo 8N1 sin control de flujo, habilita la
 * interrupción NVIC e inicia la recepción por interrupción del primer byte.
 *
 * @return true si la inicialización fue exitosa, false en caso contrario
 */
bool_t uart_init(){
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
		uart_initPrint(buffConfig, BUFFER_LENGTH);

		sprintf((char*)buffConfig,"BaudRate: %ld \r\n",apiUartInstance.Init.BaudRate);
		uart_initPrint(buffConfig, BUFFER_LENGTH);

		sprintf((char*)buffConfig,"Longitud de Palabra: 8 BITS\r\n");
		uart_initPrint(buffConfig, BUFFER_LENGTH);

		sprintf((char*)buffConfig,"Bit de Paridad: NO\r\n");
		uart_initPrint(buffConfig, BUFFER_LENGTH);

		sprintf((char*)buffConfig,"Bits de Stop: 1 BIT\r\n");
		uart_initPrint(buffConfig, BUFFER_LENGTH);

		return true;
	}
}

/**
 * @brief Copia un bloque de datos al FIFO TX e inicia la transmisión.
 *
 * Inserta byte a byte en el FIFO de transmisión. Si el FIFO se llena
 * antes de completar la copia, inicia la transmisión con los datos que
 * se hayan logrado encolar y retorna false.
 *
 * @param pstring Puntero al bloque de datos a transmitir
 * @param size Cantidad de bytes a transmitir
 * @return true si todos los bytes fueron encolados, false si el FIFO se llenó
 */
static bool_t uart_pushTxBuffer(uint8_t * pstring, uint16_t size){
	for(uint16_t i=0;i<size;i++){
		if(!uart_txPush(pstring[i])){
			// aseguramos que lo que sí entró al FIFO arranque a transmitirse
			uart_startTxIT();
			return false;
		}
	}
	uart_startTxIT();
	return true;
}

/**
 * @brief Envía una cadena de caracteres terminada en '\\0' por UART.
 *
 * Calcula la longitud del string y lo encola en el FIFO TX. Si falla,
 * reintenta hasta UART_MAX_TRANSMIT_ATTEMPTS veces.
 *
 * @param pstring Puntero al string terminado en nulo a enviar
 */
void uart_sendString(uint8_t * pstring){
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

	// Contar la longitud del string hasta el carácter nulo (sin desbordar el buffer)
	for(stringCharCounter=0;stringCharCounter<UART_MAX_STRING_LENGTH;stringCharCounter++) {
		if(pstring[stringCharCounter]== '\0') {
			stringLengthToSend = stringCharCounter;
			if(stringLengthToSend<UART_MIN_STRING_LENGTH) {
				return;
			}
			// Intentar enviar el string por UART, reintentando hasta el número máximo de intentos
			for(attemptCounter=0;attemptCounter<UART_MAX_TRANSMIT_ATTEMPTS;attemptCounter++) {
				if(uart_pushTxBuffer(pstring,stringLengthToSend)) {
					return;
				}
			}
			return;
		}
	}
}

/**
 * @brief Habilita o deshabilita el eco (loopback) de RX hacia TX.
 *
 * @param enable true para habilitar loopback, false para deshabilitarlo
 */
void uart_setLoopback(bool_t enable){
	loopbackEnable = enable;
}

/**
 * @brief Consulta si el eco (loopback) está habilitado.
 *
 * @return true si el loopback está activo, false en caso contrario
 */
bool_t uart_getLoopback(void){
	return loopbackEnable;
}

/**
 * @brief Envía una cadena de caracteres de tamaño específico por UART.
 *
 * Valida el rango de tamaño permitido y encola los datos en el FIFO TX.
 * Si falla, reintenta hasta UART_MAX_TRANSMIT_ATTEMPTS veces.
 *
 * @param pstring Puntero al buffer con los datos a enviar
 * @param size Cantidad de bytes a enviar
 */
void uart_sendStringSize(uint8_t * pstring, uint16_t size){
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
			if(uart_pushTxBuffer(pstring,size)) {
				return;
			}
		}
		return;
	}
}

/**
 * @brief Recibe una cadena de caracteres de tamaño específico por UART (bloqueante).
 *
 * Utiliza HAL_UART_Receive en modo bloqueante con timeout. Si la recepción
 * es exitosa, activa el flag de nueva data.
 *
 * @param pstring Puntero al buffer donde se almacenará la cadena recibida
 * @param size Cantidad de bytes a recibir
 */
void uart_receiveStringSize(uint8_t * pstring, uint16_t size){
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
