/*
 * API_delay.h
 *
 *  Created on: 19 mar 2026
 *      Author: nicol
 */

#ifndef API_INC_API_DELAY_H_
#define API_INC_API_DELAY_H_

#include <stdint.h>
#include <stdbool.h>

/* USER CODE BEGIN Private defines */
typedef uint32_t tick_t; // Qué biblioteca se debe incluir para que esto compile?
//stdint.h
typedef bool bool_t;	  // Qué biblioteca se debe incluir para que esto compile?
//stdbool.h

typedef struct{
   tick_t startTime;
   tick_t duration;
   bool_t running;
} delay_t;

void delayInit( delay_t * delay, tick_t duration );
bool_t delayRead( delay_t * delay );
void delayWrite( delay_t * delay, tick_t duration );
bool_t delayIsRunning( delay_t* delay );


#endif /* API_INC_API_DELAY_H_ */
