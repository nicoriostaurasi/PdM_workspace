/*
 * API_debounce.c
 *
 *  Created on: 29 mar 2026
 *      Author: nicol
 */

#include <string.h>

#include "API_debounce.h"
#include "main.h"
#include "stm32f4xx_hal.h"


/** @brief Estados de la máquina de estados del debounce */
typedef enum{
BUTTON_UP,
BUTTON_FALLING,
BUTTON_DOWN,
BUTTON_RAISING,
} debounceState_t;


/** @brief Debouncer FSM estado*/
static debounceState_t _debounceFsmState = BUTTON_UP;
/** @brief Delay para el debounce */
static delay_t debounceDelay;
/** @brief Bandera para indicar si la tecla fue presionada */
static bool_t _keyPressedFlag = false; 

/** @brief Función que verifica el estado del botón sin debounce
  * @retval true si el botón está presionado, false si no lo está
  */
static bool checkButtonStatusPressedRaw(void);

/** @brief Inicializa la máquina de estados del debounce */
void debounce_fsmInit(){
  // Inicializo el delay para el debounce y la máquina de estados del debounce
  memset(&debounceDelay, 0 ,sizeof(debounceDelay));
  delay_init(&debounceDelay, DEBOUNCER_SAMPLE_RATE);
  _debounceFsmState = BUTTON_UP;
}

/** @brief Actualiza la máquina de estados del debounce */
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

/** @brief Lee si la tecla fue presionado, si devuelve true resetea el valor
  * @retval true si el botón fue presionado, false si no lo fue
  */
bool_t debounce_readKey(void){
	// Si la bandera de tecla presionada esta seteada, se resetea y se devuelve true
	if(_keyPressedFlag){
		_keyPressedFlag = false;
		return true;
	}
	return false;
}


/** @brief Función que verifica el estado del botón sin debounce */
static bool checkButtonStatusPressedRaw(){
	return (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) != GPIO_PIN_SET);
}
