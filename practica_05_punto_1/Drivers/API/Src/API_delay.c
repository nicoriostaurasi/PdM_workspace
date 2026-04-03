/*
 * API_delay.c
 *
 *  Created on: 19 mar 2026
 *      Author: nicol
 */
#include "API_delay.h"
#include "stm32f4xx_hal.h"

/**
  * @brief  Initializes a delay structure
  * @param  delay: Pointer to the delay structure
  * @param  duration: Duration of the delay
  * @retval None
  */
void delayInit( delay_t * delay, tick_t duration ){
	// Inicializo el delay con la duración dada y lo pongo como no corriendo
  if(delay != NULL) {
    delay->duration = duration;
    delay->running = false;
  }
}

/**
  * @brief  Reads the status of a delay
  * @param  delay: Pointer to the delay structure
  * @retval bool_t: true if the delay has completed, false otherwise
  */
bool_t delayRead( delay_t * delay ){
  // Si el delay es NULL, no puedo hacer nada, devuelvo false
  if(delay == NULL) {
    return false;
  }

  // Si el delay no está corriendo, lo inicio y guardo el tiempo de inicio
  if(!delay->running) {
    // Inicio el delay y guardo el tiempo de inicio
		delay->running = true;
		delay->startTime = HAL_GetTick();
	} else {
		tick_t tiempoActual = HAL_GetTick();
		tick_t diferencia = tiempoActual - delay->startTime;

    // Si la diferencia entre el tiempo actual y el tiempo de inicio es mayor o igual a la duración del delay, lo detengo y devuelvo true
		if(diferencia >= delay->duration){
			delay->running = false;
			return true;
		}
		return false;
	}
	return false;
}

/**
  * @brief  Writes a new duration to an existing delay
  * @param  delay: Pointer to the delay structure
  * @param  duration: New duration for the delay
  * @retval None
  */
void delayWrite( delay_t * delay, tick_t duration){
  // Si el delay es NULL, no puedo hacer nada
  if(delay == NULL) {
    return;
  }
  delay->duration = duration;
}

/**
  * @brief  Check if the delay is active
  * @param  delay: Pointer to the delay structure
  * @retval bool_t: true if the delay is running, false otherwise
  */
bool_t delayIsRunning( delay_t* delay ){
	  // Si el delay es NULL, no puedo hacer nada, devuelvo false
	  if(delay == NULL) {
	    return false;
	  }
	  return (delay->running);
}


