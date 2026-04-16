/*
 * API_adxl345.h
 *
 *  Created on: 12 abr 2026
 *      Author: nicol
 */

#ifndef API_INC_API_ADXL345_H_
#define API_INC_API_ADXL345_H_

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    int16_t x;
    int16_t y;
    int16_t z;
} adxl345_raw_t;

bool adxl345_init(void);
bool adxl345_readDeviceId(uint8_t *devid);
bool adxl345_readRawAcceleration(adxl345_raw_t *raw);

/** @brief Chequeo de salud no invasivo: lee el DEVID y lo compara con el valor esperado
 *  @return true si responde al bus I2C y el DEVID es correcto, false en cualquier otro caso
 */
bool adxl345_isAlive(void);

#endif /* API_INC_API_ADXL345_H_ */
