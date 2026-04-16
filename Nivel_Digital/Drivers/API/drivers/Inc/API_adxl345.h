/**
 * @file    API_adxl345.h
 * @brief   Driver del acelerómetro ADXL345 (I2C).
 *
 * @date    12 abr 2026
 * @author  nicol
 */

#ifndef API_INC_API_ADXL345_H_
#define API_INC_API_ADXL345_H_

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief   Lectura raw de aceleración en cuentas del ADC del sensor.
 */
typedef struct
{
    int16_t x;      /**< Componente X en cuentas */
    int16_t y;      /**< Componente Y en cuentas */
    int16_t z;      /**< Componente Z en cuentas */
} adxl345_raw_t;

/**
 * @brief   Configura el sensor: verifica presencia, setea rango +/-2g, FULL_RES
 *          y pasa a modo medición.
 *
 * @return  true si la inicialización fue exitosa, false si el sensor no
 *          responde, el DEVID no coincide o falla alguna escritura.
 */
bool adxl345_init(void);

/**
 * @brief   Lee el registro DEVID del sensor (WHO_AM_I).
 *
 * @param   devid   Puntero al byte de salida.
 * @return  true si la lectura fue exitosa, false en caso contrario.
 */
bool adxl345_readDeviceId(uint8_t *devid);

/**
 * @brief   Lee las 6 posiciones de datos crudos de aceleración (X, Y, Z).
 *
 * @param   raw     Puntero a estructura de salida con los valores crudos.
 * @return  true si la lectura fue exitosa, false si @p raw es NULL o hubo
 *          error en el bus.
 */
bool adxl345_readRawAcceleration(adxl345_raw_t *raw);

/**
 * @brief   Chequeo de salud no invasivo: lee el DEVID y lo compara con el valor
 *          esperado.
 *
 * @return  true si responde al bus I2C y el DEVID es correcto, false en
 *          cualquier otro caso.
 */
bool adxl345_isAlive(void);

#endif /* API_INC_API_ADXL345_H_ */
