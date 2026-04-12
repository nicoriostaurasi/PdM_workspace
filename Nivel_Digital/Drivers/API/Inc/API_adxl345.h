/*
 * API_adxl345.h
 *
 *  Created on: 12 abr 2026
 *      Author: nicol
 */

#ifndef API_INC_API_ADXL345_H_
#define API_INC_API_ADXL345_H_

#include "API_i2c.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    int16_t x;
    int16_t y;
    int16_t z;
} ADXL345_Raw_t;

typedef struct
{
    float x;
    float y;
    float z;
} ADXL345_AccelG_t;

bool adxl345_init(void);
bool adxl345_readReg(uint8_t reg, uint8_t *value);
bool adxl345_readDeviceId(uint8_t *devid);
bool adxl345_readGAccel(ADXL345_AccelG_t *accel);

#endif /* API_INC_API_ADXL345_H_ */
