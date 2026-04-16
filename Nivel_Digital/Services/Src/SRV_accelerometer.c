/**
 * @file  SRV_accelerometer.c
 * @brief Capa de abstraccion del acelerometro; delega en el driver ADXL345.
 * @date 13 abr 2026
 * @author Ing. Nicolás Gabriel Rios Taurasi
 */

#include <stdlib.h>

#include "PER_adxl345.h"
#include "SRV_accelerometer.h"

/**
 * @brief Inicializa el sensor acelerometro delegando en adxl345_init.
 * @return true si la inicializacion fue exitosa, false en caso contrario.
 */
bool accelerometer_initSensor(void){
	return adxl345_init();
}

/**
 * @brief Lee la aceleracion del sensor y la convierte a unidades de gravedad (g).
 *
 * Obtiene la lectura cruda del ADXL345 y divide cada eje por 256 para
 * obtener el valor en g (resolucion FULL_RES a +/-2 g = 256 LSB/g).
 *
 * @param accel Puntero a la estructura donde se almacenan los valores en g.
 * @return true si la lectura fue exitosa, false si el puntero es NULL o fallo la lectura.
 */
bool accelerometer_readAccelerationG(adxl345_accelG_t *accel)
{
	bool ret;
	adxl345_raw_t raw;

    if (accel == NULL)
    {
        return false;
    }

    ret = adxl345_readRawAcceleration(&raw);
    if (!ret)
    {
        return false;
    }

    accel->x = (float)raw.x / 256.0f;
    accel->y = (float)raw.y / 256.0f;
    accel->z = (float)raw.z / 256.0f;

    return true;
}
