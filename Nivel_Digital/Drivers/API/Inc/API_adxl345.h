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

bool adxl345_init(void);
bool adxl345_readReg(uint8_t reg, uint8_t *value);
bool adxl345_readDeviceId(uint8_t *devid);

#endif /* API_INC_API_ADXL345_H_ */
