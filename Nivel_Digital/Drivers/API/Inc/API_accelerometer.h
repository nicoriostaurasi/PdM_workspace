/*
 * API_accelerometer.h
 *
 *  Created on: 13 abr 2026
 *      Author: nicol
 */

#ifndef API_INC_API_ACCELEROMETER_H_
#define API_INC_API_ACCELEROMETER_H_

#include <stdbool.h>

typedef struct
{
    float x;
    float y;
    float z;
} adxl345_accelG_t;


bool accelerometer_readAccelerationG(adxl345_accelG_t *accel);
bool accelerometer_initSensor(void);

#endif /* API_INC_API_ACCELEROMETER_H_ */
