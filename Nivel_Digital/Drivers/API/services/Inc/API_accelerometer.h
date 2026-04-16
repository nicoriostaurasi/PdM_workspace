/**
 * @file    API_accelerometer.h
 * @brief   Capa de abstracción del acelerómetro: expone lecturas en unidades de g.
 *
 * @date    13 abr 2026
 * @author  nicol
 */

#ifndef API_INC_API_ACCELEROMETER_H_
#define API_INC_API_ACCELEROMETER_H_

#include <stdbool.h>

/**
 * @brief   Aceleración expresada en unidades de g (1 g = 9.81 m/s^2).
 */
typedef struct
{
    float x;    /**< Componente X en g */
    float y;    /**< Componente Y en g */
    float z;    /**< Componente Z en g */
} adxl345_accelG_t;


/**
 * @brief   Lee la aceleración actual del sensor y la devuelve en unidades de g.
 *
 * @param   accel   Puntero a la estructura de salida donde se escriben X/Y/Z en g.
 * @return  true si la lectura fue exitosa, false si el puntero es NULL o hubo
 *          error en la comunicación con el sensor.
 */
bool accelerometer_readAccelerationG(adxl345_accelG_t *accel);

/**
 * @brief   Inicializa el sensor de aceleración y lo deja listo para mediciones.
 *
 * @return  true si la inicialización fue exitosa, false en caso contrario.
 */
bool accelerometer_initSensor(void);

#endif /* API_INC_API_ACCELEROMETER_H_ */
