/*
 * API_accelerometer.h
 *
 *  Created on: 13 abr 2026
 *      Author: nicol
 */

#ifndef API_INC_API_ACCELEROMETER_H_
#define API_INC_API_ACCELEROMETER_H_

typedef struct
{
    float x;
    float y;
    float z;
} ADXL345_AccelG_t;


bool accelerometer_readAccelerationG(ADXL345_AccelG_t *accel);
bool accelerometer_initSensor(void);

#endif /* API_INC_API_ACCELEROMETER_H_ */
