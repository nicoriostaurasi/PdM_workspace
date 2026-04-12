/*
 * API_adxl345.c
 *
 *  Created on: 12 abr 2026
 *      Author: nicol
 */
#include "API_adxl345.h"
#include <stdlib.h>
#define ADXL345_I2C_ADDRESS   (0x1D << 1)

#define ADXL345_REG_DEVID         0x00
#define ADXL345_REG_THRESH_TAP    0x1D
#define ADXL345_REG_OFSX          0x1E
#define ADXL345_REG_OFSY          0x1F
#define ADXL345_REG_OFSZ          0x20
#define ADXL345_REG_BW_RATE       0x2C
#define ADXL345_REG_POWER_CTL     0x2D
#define ADXL345_REG_INT_ENABLE    0x2E
#define ADXL345_REG_INT_MAP       0x2F
#define ADXL345_REG_INT_SOURCE    0x30
#define ADXL345_REG_DATA_FORMAT   0x31
#define ADXL345_REG_DATA_BASE     0x32
#define ADXL345_REG_DATAX0        0x32
#define ADXL345_REG_DATAX1        0x33
#define ADXL345_REG_DATAY0        0x34
#define ADXL345_REG_DATAY1        0x35
#define ADXL345_REG_DATAZ0        0x36
#define ADXL345_REG_DATAZ1        0x37
#define ADXL345_REG_FIFO_CTL      0x38
#define ADXL345_REG_FIFO_STATUS   0x39
#define BYTE_SHIFT 				  0x08
#define ADXL345_DEVID_VALUE       0xE5

#define ADXL345_I2C_TIMEOUT 	  1000

bool adxl345_readReg(uint8_t reg, uint8_t *value){
	return i2c_memRead(ADXL345_I2C_ADDRESS, reg, value, 1, ADXL345_I2C_TIMEOUT);
}

bool adxl345_readRegs(uint8_t reg, uint8_t *buffer, uint16_t len){
	return i2c_memRead(ADXL345_I2C_ADDRESS, reg, buffer, len, ADXL345_I2C_TIMEOUT);
}

bool adxl345_writeReg(uint8_t reg, uint8_t value){
	return i2c_memWrite(ADXL345_I2C_ADDRESS, reg, &value, 1, ADXL345_I2C_TIMEOUT);
}

bool adxl345_readDeviceId(uint8_t *devid){
    return adxl345_readReg(ADXL345_REG_DEVID, devid);
}


bool adxl345_init(void){
    bool ret;
    uint8_t devId = 0;

    ret = i2c_isDeviceReady(ADXL345_I2C_ADDRESS, 2, ADXL345_I2C_TIMEOUT);
    if (!ret){
        return false;
    }

    ret = adxl345_readDeviceId(&devId);
    if (!ret){
        return false;
    }

    if (devId != ADXL345_DEVID_VALUE){
        return false;
    }

    /* Standby antes de configurar */
    ret = adxl345_writeReg(ADXL345_REG_POWER_CTL, 0x00);
    if (!ret){
    	return false;
    }

    /* Data rate = 100 Hz */
    ret = adxl345_writeReg(ADXL345_REG_BW_RATE, 0x0A);
    if (!ret){
    	return false;
    }

    /* FULL_RES = 1, rango ±2g */
    ret = adxl345_writeReg(ADXL345_REG_DATA_FORMAT, 0x08);
    if (!ret){
    	return false;
    }

    /* Measurement mode */
    ret = adxl345_writeReg(ADXL345_REG_POWER_CTL, 0x08);
    if (!ret){
    	return false;
    }

	return true;
}

bool adxl345_readRaw(ADXL345_Raw_t *raw)
{
	bool ret;
    uint8_t rawData[6];

    if (raw == NULL){
        return false;
    }

    ret = adxl345_readRegs(ADXL345_REG_DATA_BASE, rawData, 6);

    if (!ret){
        return false;
    }

    raw->x = (int16_t)((rawData[1] << BYTE_SHIFT) | rawData[0]);
    raw->y = (int16_t)((rawData[3] << BYTE_SHIFT) | rawData[2]);
    raw->z = (int16_t)((rawData[5] << BYTE_SHIFT) | rawData[4]);

    return true;
}


bool adxl345_readGAccel(ADXL345_AccelG_t *accel)
{
	bool ret;
	ADXL345_Raw_t raw;

    if (accel == NULL)
    {
        return false;
    }

    ret = adxl345_readRaw(&raw);
    if (!ret)
    {
        return false;
    }

    accel->x = (float)raw.x / 256.0f;
    accel->y = (float)raw.y / 256.0f;
    accel->z = (float)raw.z / 256.0f;

    return true;
}
