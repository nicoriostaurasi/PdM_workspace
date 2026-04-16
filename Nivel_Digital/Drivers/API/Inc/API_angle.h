/*
 * API_angle.h
 *
 *  Created on: 13 abr 2026
 *      Author: nicol
 */

#ifndef API_INC_API_ANGLE_H_
#define API_INC_API_ANGLE_H_

#include "API_accelerometer.h"

typedef struct
{
    float pitch;
    float roll;
} angles_t;

angles_t angle_fromAcceleration(adxl345_accelG_t* pAccel);

#endif /* API_INC_API_ANGLE_H_ */
