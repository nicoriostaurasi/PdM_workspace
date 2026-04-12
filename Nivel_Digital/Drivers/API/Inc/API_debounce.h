/*
 * API_debounce.h
 *
 *  Created on: 29 mar 2026
 *      Author: nicol
 */

#ifndef API_INC_API_DEBOUNCE_H_
#define API_INC_API_DEBOUNCE_H_

#include <API_delay.h>

#define MS_TO_TICK 1
#define DEBOUNCER_SAMPLE_RATE (20*MS_TO_TICK)
#define DEBOUNCER_TIME (40*MS_TO_TICK)
#define DEBOUNCE_COUNTER_MAX (DEBOUNCER_TIME/DEBOUNCER_SAMPLE_RATE)

/** @brief Inicializa la máquina de estados del debounce
  * @retval none
  */
void debounceFSM_init(void);

/** @brief Actualiza la máquina de estados del debounce
  * @retval none
  */
void debounceFSM_update(void);

/** @brief Lee si la tecla fue presionado, si devuelve true resetea el valor
  * @retval true si el botón fue presionado
  */
bool_t readKey(void);


#endif /* API_INC_API_DEBOUNCE_H_ */
