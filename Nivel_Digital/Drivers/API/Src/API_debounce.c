/** @file   API_debounce.c
 *  @brief  Implementación de la FSM de debounce por software para el botón de usuario.
 *
 *  @date   29 mar 2026
 *  @author Nicolás Rios Taurasi
 */

#include <string.h>

#include "API_debounce.h"
#include "main.h"
#include "stm32f4xx_hal.h"


/** @brief Estados de la máquina de estados del debounce */
typedef enum{
	BUTTON_UP,      /**< Botón liberado (estado estable) */
	BUTTON_FALLING, /**< Transición detectada hacia presionado, pendiente de confirmar */
	BUTTON_DOWN,    /**< Botón presionado (estado estable) */
	BUTTON_RAISING, /**< Transición detectada hacia liberado, pendiente de confirmar */
} debounceState_t;


/** @brief Estado actual de la FSM de debounce */
static debounceState_t _debounceFsmState = BUTTON_UP;
/** @brief Delay para el muestreo periódico del debounce */
static delay_t debounceDelay;
/** @brief Bandera interna que indica si se detectó una pulsación válida */
static bool_t _keyPressedFlag = false;

/** @brief  Verifica el estado eléctrico del botón sin filtrado de debounce.
 *  @return true si el botón está físicamente presionado, false si está liberado.
 */
static bool checkButtonStatusPressedRaw(void);

/** @brief Inicializa la máquina de estados del debounce y su timer interno.
 *
 *  Pone la FSM en BUTTON_UP y configura el delay de muestreo.
 */
void debounce_fsmInit(){
  // Inicializo el delay para el debounce y la máquina de estados del debounce
  memset(&debounceDelay, 0 ,sizeof(debounceDelay));
  delay_init(&debounceDelay, DEBOUNCER_SAMPLE_RATE);
  _debounceFsmState = BUTTON_UP;
}

/** @brief Avanza la máquina de estados del debounce.
 *
 *  Debe llamarse periódicamente desde el bucle principal. Utiliza contadores
 *  internos para confirmar las transiciones (falling/raising) antes de
 *  cambiar de estado estable.
 */
void debounce_fsmUpdate(){

  static uint8_t fallingStateCounter = 0;
  static uint8_t risingStateCounter = 0;

  // Verifico que el tiempo de muestreo del debounce haya transcurrido
  if(!delay_read(&debounceDelay)){
		  return;
	}

  // Switch de la máquina de estados del debounce
  switch(_debounceFsmState){
	case BUTTON_UP: {

    // Si el boton esta presionado, se pasa al estado de BUTTON_FALLING
		if(checkButtonStatusPressedRaw()) {
			_debounceFsmState = BUTTON_FALLING;
			fallingStateCounter=0;
		}
		break;
	}

	case BUTTON_FALLING: {

    // Si el boton sigue presionado, se incrementa el contador de estados de caída, sino se vuelve al estado de BUTTON_UP
		if(checkButtonStatusPressedRaw()){
			fallingStateCounter++;
		}
		else{
			fallingStateCounter=0;
			_debounceFsmState = BUTTON_UP;
		}

    // Si el contador de estados de caída supera el máximo, se pasa al estado de BUTTON_DOWN
		if(fallingStateCounter>=DEBOUNCE_COUNTER_MAX){
			fallingStateCounter=0;
			_debounceFsmState = BUTTON_DOWN;
			_keyPressedFlag = true; // Se setea la bandera de tecla presionada
		}

		break;
	}

	case BUTTON_DOWN: {

    // Si el boton esta liberado, se pasa al estado de BUTTON_RAISING
		if(!checkButtonStatusPressedRaw()) {
			_debounceFsmState = BUTTON_RAISING;
			risingStateCounter = 0;
		}
		break;
	}

	case BUTTON_RAISING: {

    // Si el boton sigue liberado, se incrementa el contador de estados de subida, sino se vuelve al estado de BUTTON_DOWN
		if(!checkButtonStatusPressedRaw()){
			risingStateCounter++;
		}
		else{
			risingStateCounter=0;
			_debounceFsmState = BUTTON_DOWN;
		}

    // Si el contador de estados de subida supera el máximo, se pasa al estado de BUTTON_UP
		if(risingStateCounter>=DEBOUNCE_COUNTER_MAX){
			risingStateCounter=0;
			_debounceFsmState = BUTTON_UP;
		}
		break;
	}

	default: {
    // En caso de que el estado sea inválido, se vuelve al estado de BUTTON_UP
		_debounceFsmState = BUTTON_UP;
		break;
	}

	}
}

/** @brief  Consulta si se detectó una pulsación desde la última llamada.
 *
 *  Si devuelve true, la bandera interna se resetea automáticamente (one-shot).
 *
 *  @return true si hubo una pulsación pendiente, false en caso contrario.
 */
bool_t debounce_readKey(void){
	// Si la bandera de tecla presionada esta seteada, se resetea y se devuelve true
	if(_keyPressedFlag){
		_keyPressedFlag = false;
		return true;
	}
	return false;
}


/** @brief  Verifica el estado eléctrico del botón sin filtrado de debounce.
 *  @return true si el botón está físicamente presionado, false si está liberado.
 */
static bool checkButtonStatusPressedRaw(){
	return (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) != GPIO_PIN_SET);
}
