/**
 * @file    API_debounce.h
 * @brief   FSM de debounce por software para el botón de usuario.
 *
 * @date    29 mar 2026
 * @author  nicol
 */

#ifndef API_INC_API_DEBOUNCE_H_
#define API_INC_API_DEBOUNCE_H_

#include "API_delay.h"

#define MS_TO_TICK              1                                   /**< Factor de conversión ms a ticks */
#define DEBOUNCER_SAMPLE_RATE   (20*MS_TO_TICK)                     /**< Período de muestreo de la FSM, en ms */
#define DEBOUNCER_TIME          (40*MS_TO_TICK)                     /**< Tiempo total de debounce, en ms */
#define DEBOUNCE_COUNTER_MAX    (DEBOUNCER_TIME/DEBOUNCER_SAMPLE_RATE) /**< Muestras consecutivas requeridas para validar transición */

/**
 * @brief   Inicializa la máquina de estados del debounce y su timer interno.
 */
void debounce_fsmInit(void);

/**
 * @brief   Avanza la máquina de estados del debounce. Debe llamarse
 *          periódicamente desde el bucle principal.
 */
void debounce_fsmUpdate(void);

/**
 * @brief   Consulta si se detectó una pulsación de tecla desde la última llamada.
 *
 *          Si devuelve true, la bandera interna se resetea (one-shot).
 *
 * @return  true si hubo una pulsación pendiente, false en caso contrario.
 */
bool_t debounce_readKey(void);


#endif /* API_INC_API_DEBOUNCE_H_ */
