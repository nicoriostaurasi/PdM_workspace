/**
 * @file    API_i2c.h
 * @brief   Wrapper delgado sobre HAL I2C1 para lecturas/escrituras por registro.
 *
 * @date    12 abr 2026
 * @author  nicol
 */

#ifndef API_INC_API_I2C_H_
#define API_INC_API_I2C_H_

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief   Inicializa el periférico I2C1 (100 kHz, modo 7 bits).
 *
 * @return  true si la inicialización fue exitosa, false en caso contrario.
 */
bool i2c_init1(void);

/**
 * @brief   Verifica si un dispositivo responde en la dirección indicada.
 *
 * @param   DevAddress  Dirección I2C del dispositivo (desplazada 1 bit a la izquierda).
 * @param   Trials      Cantidad de intentos antes de declarar error.
 * @param   Timeout     Timeout por intento, en ms.
 * @return  true si el dispositivo respondió al ACK, false en caso contrario.
 */
bool i2c_isDeviceReady(uint16_t DevAddress, uint32_t Trials, uint32_t Timeout);

/**
 * @brief   Lee una o más posiciones de memoria de un dispositivo I2C.
 *
 * @param   dev_addr    Dirección I2C del dispositivo.
 * @param   reg         Registro/subdirección desde donde leer.
 * @param   value       Buffer de salida donde se escribe la lectura.
 * @param   size        Cantidad de bytes a leer.
 * @param   timeout     Timeout en ms.
 * @return  true si la lectura fue exitosa, false en caso contrario o si @p value es NULL.
 */
bool i2c_memRead(uint16_t dev_addr, uint8_t reg, uint8_t* value, uint8_t size, uint32_t timeout);

/**
 * @brief   Escribe una o más posiciones de memoria de un dispositivo I2C.
 *
 * @param   dev_addr    Dirección I2C del dispositivo.
 * @param   reg         Registro/subdirección a escribir.
 * @param   value       Buffer con los datos a enviar.
 * @param   size        Cantidad de bytes a escribir.
 * @param   timeout     Timeout en ms.
 * @return  true si la escritura fue exitosa, false en caso contrario o si @p value es NULL.
 */
bool i2c_memWrite(uint16_t dev_addr, uint8_t reg, uint8_t* value, uint8_t size, uint32_t timeout);

#endif /* API_INC_API_I2C_H_ */
