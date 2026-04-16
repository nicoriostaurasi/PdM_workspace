/*
 * API_i2c.h
 *
 *  Created on: 12 abr 2026
 *      Author: nicol
 */

#ifndef API_INC_API_I2C_H_
#define API_INC_API_I2C_H_

#include <stdbool.h>
#include <stdint.h>

bool i2c_init1(void);
bool i2c_isDeviceReady(uint16_t DevAddress, uint32_t Trials, uint32_t Timeout);
bool i2c_memRead(uint16_t dev_addr, uint8_t reg, uint8_t* value, uint8_t size, uint32_t timeout);
bool i2c_memWrite(uint16_t dev_addr, uint8_t reg, uint8_t* value, uint8_t size, uint32_t timeout);

#endif /* API_INC_API_I2C_H_ */
