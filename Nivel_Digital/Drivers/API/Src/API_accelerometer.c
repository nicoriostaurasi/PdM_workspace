/*
 * API_accelerometer.c
 *
 *  Created on: 13 abr 2026
 *      Author: nicol
 */

#include "API_adxl345.h"
#include "API_accelerometer.h"
#include <stdio.h>

bool accelerometer_initSensor(void){
	return adxl345_init();
}

bool accelerometer_readAccelerationG(ADXL345_AccelG_t *accel)
{
	bool ret;
	ADXL345_Raw_t raw;

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
