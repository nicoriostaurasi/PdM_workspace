/**
 * @file API_angle.c
 * @brief Calculo de angulos pitch y roll a partir de aceleracion, con promedio circular.
 * @date 13 abr 2026
 * @author Nicolas Rios Taurasi
 */

#include <math.h>
#include <stdint.h>

#include "API_angle.h"

#define PI                  3.14159265358979323846f
#define ANGLE_AVG_SAMPLES   3U

/**
 * @brief Convierte un angulo de radianes a grados.
 * @param rad Angulo en radianes.
 * @return Angulo convertido a grados.
 */
static float rad_to_deg(float rad){
    return rad * (180.0f / PI);
}

/**
 * @brief Calcula los angulos pitch y roll a partir de la aceleracion en g.
 *
 * Utiliza un buffer circular de ANGLE_AVG_SAMPLES muestras para promediar
 * los angulos y suavizar la salida. Cada llamada agrega la muestra actual
 * al buffer y devuelve el promedio de todas las muestras almacenadas.
 *
 * @param pAccel Puntero a la estructura con la aceleracion en g (ejes x, y, z).
 * @return Estructura angles_t con los angulos pitch y roll promediados en grados.
 *         Si pAccel es NULL, retorna angulos en cero.
 */
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
