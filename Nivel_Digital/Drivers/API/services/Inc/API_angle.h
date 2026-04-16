/**
 * @file    API_angle.h
 * @brief   Conversión de aceleración a ángulos pitch/roll.
 *
 * @date    13 abr 2026
 * @author  nicol
 */

#ifndef API_INC_API_ANGLE_H_
#define API_INC_API_ANGLE_H_

#include "API_accelerometer.h"

/**
 * @brief   Ángulos pitch y roll expresados en grados.
 */
typedef struct
{
    float pitch;    /**< Inclinación sobre eje X, en grados */
    float roll;     /**< Inclinación sobre eje Y, en grados */
} angles_t;

/**
 * @brief   Convierte una muestra de aceleración en ángulos pitch/roll promediados.
 *
 *          Internamente mantiene un buffer circular con las últimas muestras y
 *          devuelve el promedio, para atenuar ruido de alta frecuencia.
 *
 * @param   pAccel  Puntero a la aceleración de entrada (en g).
 * @return  Estructura con pitch y roll en grados. Si @p pAccel es NULL devuelve
 *          ángulos en cero.
 */
angles_t angle_fromAcceleration(adxl345_accelG_t* pAccel);

#endif /* API_INC_API_ANGLE_H_ */
