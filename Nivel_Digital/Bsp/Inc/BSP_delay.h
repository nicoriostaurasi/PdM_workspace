/**
 * @file    BSP_delay.h
 * @brief   Temporizadores por software no bloqueantes basados en HAL_GetTick().
 *
 * @date    19 mar 2026
 * @author  Ing. Nicolas Gabriel Rios Taurasi
 */

#ifndef BSP_INC_BSP_DELAY_H_
#define BSP_INC_BSP_DELAY_H_

#include <stdint.h>
#include <stdbool.h>

typedef uint32_t tick_t;        /**< Tipo entero que representa un tick del sistema (ms) */
typedef bool bool_t;            /**< Alias de bool para homogeneidad del proyecto */

/**
 * @brief   Estado interno de un temporizador por software.
 */
typedef struct{
   tick_t startTime;            /**< Tick capturado al arrancar el delay */
   tick_t duration;             /**< Duración configurada, en ticks */
   bool_t running;              /**< Bandera de delay en curso */
} delay_t;

/**
 * @brief   Inicializa un temporizador con la duración indicada.
 *
 * @param   delay       Puntero a la estructura de delay a inicializar.
 * @param   duration    Duración del delay, en ticks (ms).
 */
void delay_init( delay_t * delay, tick_t duration );

/**
 * @brief   Consulta si el temporizador cumplió su tiempo. Si no estaba corriendo
 *          lo arranca; si ya venció, lo detiene y devuelve true una sola vez.
 *
 * @param   delay   Puntero al delay.
 * @return  true si el delay venció en esta lectura, false si aún no o si el
 *          puntero es NULL.
 */
bool_t delay_read( delay_t * delay );

/**
 * @brief   Reescribe la duración de un delay existente sin afectar su estado.
 *
 * @param   delay       Puntero al delay.
 * @param   duration    Nueva duración, en ticks (ms).
 */
void delay_write( delay_t * delay, tick_t duration );

/**
 * @brief   Consulta si el temporizador está corriendo.
 *
 * @param   delay   Puntero al delay.
 * @return  true si está corriendo, false en caso contrario o si el puntero es NULL.
 */
bool_t delay_isRunning( delay_t* delay );

/**
 * @brief   Espera bloqueante durante la cantidad de milisegundos indicada.
 *
 * @param   ms  Tiempo de espera en milisegundos.
 */
void delay_blocking( tick_t ms );

#endif /* BSP_INC_BSP_DELAY_H_ */
