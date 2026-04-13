/*
 * API_angle.c
 *
 *  Created on: 13 abr 2026
 *      Author: nicol
 */

#include "API_angle.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdint.h>

#define PI 3.14159265358979323846f

static float rad_to_deg(float rad){
    return rad * (180.0f / PI);
}

angles_t convertAccelerationToAngle(ADXL345_AccelG_t* pAccel){
	static angles_t currentAngleBuffer[3] = {0};
	angles_t averageAngle;
	float averagePitch = 0.0;
	float averageRoll = 0.0;

	if(pAccel==NULL){
		return currentAngleBuffer[0];
	}

	currentAngleBuffer[0].pitch = rad_to_deg(
        atan2f(pAccel->x, sqrtf((pAccel->y * pAccel->y) + (pAccel->z * pAccel->z)))
    );

	currentAngleBuffer[0].roll = rad_to_deg(
        atan2f(pAccel->y, sqrtf((pAccel->x * pAccel->x) + (pAccel->z * pAccel->z)))
    );

	for(uint8_t i=0;i<3;i++){
		averagePitch+=currentAngleBuffer[i].pitch;
		averageRoll+=currentAngleBuffer[i].roll;
	}

	for(uint8_t i=0;i<2;i++){
		memcpy(&currentAngleBuffer[i+1],&currentAngleBuffer[i],sizeof(angles_t));
	}


	averageAngle.pitch = averagePitch/3.0;
    averageAngle.roll = averageRoll/3.0;

	return averageAngle;
}
