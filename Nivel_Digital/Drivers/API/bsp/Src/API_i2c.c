/**
 * @file API_i2c.c
 * @brief Capa de abstraccion del periferico I2C1 sobre HAL.
 * @date 12 abr 2026
 * @author Nicolas Rios Taurasi
 */

#include "API_i2c.h"
#include "stm32f4xx_hal.h"

/** @brief Handle interno del periferico I2C1. */
static I2C_HandleTypeDef hi2c1;

/**
 * @brief Lee datos desde un registro de un dispositivo I2C.
 * @param dev_addr Direccion I2C del dispositivo (7 bits desplazados a la izquierda).
 * @param reg Direccion del registro a leer.
 * @param value Puntero al buffer donde se almacenan los bytes leidos.
 * @param size Cantidad de bytes a leer.
 * @param timeout Tiempo maximo de espera en milisegundos.
 * @return true si la lectura fue exitosa, false si value es NULL o fallo la comunicacion.
 */
bool i2c_memRead(uint16_t dev_addr, uint8_t reg, uint8_t* value, uint8_t size, uint32_t timeout){
	if(value == NULL){
		return false;
	}

	return (HAL_I2C_Mem_Read(&hi2c1,
			dev_addr,
            reg,
            I2C_MEMADD_SIZE_8BIT,
            value,
            size,
			timeout) == HAL_OK);
}

/**
 * @brief Escribe datos en un registro de un dispositivo I2C.
 * @param dev_addr Direccion I2C del dispositivo (7 bits desplazados a la izquierda).
 * @param reg Direccion del registro a escribir.
 * @param value Puntero al buffer con los datos a escribir.
 * @param size Cantidad de bytes a escribir.
 * @param timeout Tiempo maximo de espera en milisegundos.
 * @return true si la escritura fue exitosa, false si value es NULL o fallo la comunicacion.
 */
bool i2c_memWrite(uint16_t dev_addr, uint8_t reg, uint8_t* value, uint8_t size, uint32_t timeout){
	if(value == NULL){
		return false;
	}

	return (HAL_I2C_Mem_Write(&hi2c1,
			dev_addr,
            reg,
            I2C_MEMADD_SIZE_8BIT,
            value,
            size,
			timeout) == HAL_OK);
}

/**
 * @brief Verifica si un dispositivo I2C esta listo para comunicarse.
 * @param DevAddress Direccion I2C del dispositivo.
 * @param Trials Cantidad de reintentos.
 * @param Timeout Tiempo maximo de espera en milisegundos.
 * @return true si el dispositivo responde, false en caso contrario.
 */
bool i2c_isDeviceReady(uint16_t DevAddress, uint32_t Trials, uint32_t Timeout){
	if(HAL_I2C_IsDeviceReady(&hi2c1, DevAddress, Trials, Timeout) == HAL_OK) {
		return true;
	}

	return false;
}

/**
 * @brief Inicializa el periferico I2C1 a 100 kHz en modo 7 bits.
 * @return true si la inicializacion fue exitosa, false en caso contrario.
 */
bool i2c_init1(void){
	hi2c1.Instance = I2C1;
	hi2c1.Init.ClockSpeed = 100000;
	hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
	hi2c1.Init.OwnAddress1 = 0;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

	return (HAL_I2C_Init(&hi2c1) == HAL_OK);
}
