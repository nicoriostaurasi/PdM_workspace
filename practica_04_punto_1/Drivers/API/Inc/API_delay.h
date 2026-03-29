/*
 * API_delay.h
 *
 *  Created on: 19 mar 2026
 *      Author: Ing. Nicolas Gabriel Rios Taurasi
 */

#ifndef API_INC_API_DELAY_H_
#define API_INC_API_DELAY_H_

#include <stdint.h>
#include <stdbool.h>

typedef uint32_t tick_t; // Qué biblioteca se debe incluir para que esto compile?
//stdint.h
typedef bool bool_t;	  // Qué biblioteca se debe incluir para que esto compile?
//stdbool.h

/**
 * @brief Delay structure
 */
typedef struct{
   tick_t startTime;
   tick_t duration;
   bool_t running;
} delay_t;

/**
  * @brief  Initializes a delay structure
  * @param  delay: Pointer to the delay structure
  * @param  duration: Duration of the delay
  * @retval None
  */
void delayInit( delay_t * delay, tick_t duration );

/**
  * @brief  Reads the status of a delay
  * @param  delay: Pointer to the delay structure
  * @retval bool_t: true if the delay has completed, false otherwise
  */
bool_t delayRead( delay_t * delay );

/**
  * @brief  Writes a new duration to an existing delay
  * @param  delay: Pointer to the delay structure
  * @param  duration: New duration for the delay
  * @retval None
  */
void delayWrite( delay_t * delay, tick_t duration );

/**
  * @brief  Check if the delay is active
  * @param  delay: Pointer to the delay structure
  * @retval bool_t: true if the delay is running, false otherwise
  */
bool_t delayIsRunning( delay_t* delay );


#endif /* API_INC_API_DELAY_H_ */
