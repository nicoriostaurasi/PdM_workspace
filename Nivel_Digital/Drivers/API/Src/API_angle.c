/*
 * API_angle.c
 *
 *  Created on: 13 abr 2026
 *      Author: nicol
 */

#include "API_angle.h"
#include <math.h>
#include <stdint.h>

#define PI                  3.14159265358979323846f
#define ANGLE_AVG_SAMPLES   3U

static float rad_to_deg(float rad){
    return rad * (180.0f / PI);
}

angles_t angle_fromAcceleration(adxl345_accelG_t* pAccel){
	static angles_t sampleBuffer[ANGLE_AVG_SAMPLES] = {0};
	static uint8_t writeIdx = 0;

	angles_t averageAngle = {0};
	float sumPitch = 0.0f;
	float sumRoll = 0.0f;

	if(pAccel == NULL){
		return averageAngle;
	}

	// Calculo pitch y roll de la muestra actual y la guardo en el slot circular
	sampleBuffer[writeIdx].pitch = rad_to_deg(
		atan2f(pAccel->x, sqrtf((pAccel->y * pAccel->y) + (pAccel->z * pAccel->z)))
	);
	sampleBuffer[writeIdx].roll = rad_to_deg(
		atan2f(pAccel->y, sqrtf((pAccel->x * pAccel->x) + (pAccel->z * pAccel->z)))
	);

	writeIdx = (writeIdx + 1U) % ANGLE_AVG_SAMPLES;

	// Promedio sobre las ANGLE_AVG_SAMPLES muestras guardadas
	for(uint8_t i = 0; i < ANGLE_AVG_SAMPLES; i++){
		sumPitch += sampleBuffer[i].pitch;
		sumRoll  += sampleBuffer[i].roll;
	}

	averageAngle.pitch = sumPitch / (float)ANGLE_AVG_SAMPLES;
	averageAngle.roll  = sumRoll  / (float)ANGLE_AVG_SAMPLES;

	return averageAngle;
}
