/** @file   BSP_delay.c
 *  @brief  Implementación de temporizadores por software no bloqueantes basados en HAL_GetTick().
 *
 *  @date   19 mar 2026
 *  @author Ing. Nicolás Gabriel Rios Taurasi
 */
#include "BSP_delay.h"
#include "stm32f4xx_hal.h"

/** @brief  Inicializa una estructura de delay con la duración indicada.
 *
 *  Configura la duración y deja el temporizador detenido. Si el puntero es
 *  NULL no realiza ninguna operación.
 *
 *  @param  delay     Puntero a la estructura de delay a inicializar.
 *  @param  duration  Duración del delay, en ticks (ms).
 */
void delay_init( delay_t * delay, tick_t duration ){
	// Inicializo el delay con la duración dada y lo pongo como no corriendo
  if(delay != NULL) {
    delay->duration = duration;
    delay->running = false;
  }
}

/** @brief  Consulta si el temporizador cumplió su tiempo.
 *
 *  Si el delay no estaba corriendo lo arranca capturando el tick actual.
 *  Si ya venció, lo detiene y devuelve true una sola vez.
 *
 *  @param  delay  Puntero a la estructura de delay.
 *  @return true si el delay venció en esta lectura, false si aún no venció
 *          o si el puntero es NULL.
 */
bool_t delay_read( delay_t * delay ){
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

/** @brief  Reescribe la duración de un delay existente sin afectar su estado.
 *
 *  Si el puntero es NULL no realiza ninguna operación.
 *
 *  @param  delay     Puntero a la estructura de delay.
 *  @param  duration  Nueva duración, en ticks (ms).
 */
void delay_write( delay_t * delay, tick_t duration){
  // Si el delay es NULL, no puedo hacer nada
  if(delay == NULL) {
    return;
  }
  delay->duration = duration;
}

/** @brief  Consulta si el temporizador está corriendo.
 *
 *  @param  delay  Puntero a la estructura de delay.
 *  @return true si el delay está activo, false en caso contrario o si el
 *          puntero es NULL.
 */
bool_t delay_isRunning( delay_t* delay ){
	  // Si el delay es NULL, no puedo hacer nada, devuelvo false
	  if(delay == NULL) {
	    return false;
	  }
	  return (delay->running);
}
