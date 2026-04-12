/*
 * API_digital_angle_meter.c
 *
 *  Created on: 11 abr 2026
 *      Author: nicol
 */

#include "API_digital_angle_meter.h"
#include <stdbool.h>

typedef enum {
	INIT = 0,
	IDLE,
	HANDLE_UART,
	HANDLE_BUTTON,
	READ_SENSOR,
	PROCESS_DATA,
	UPDATE_DISPLAY,
	ERROR,
} digitalAngleMeterState_t;

typedef enum {
	DIGITAL = 0,
	ANALOGIC,
} displayMode_t;

typedef struct {
	int angle1;
	int angle2;
} angle_t;

typedef struct {
	int acel_1;
	int acel_2;
} aceleration_t;


static digitalAngleMeterState_t digitalAngleMeterFsmState = INIT;
static displayMode_t displayMode = DIGITAL;

//llama al modulo de uart, y pregunta si hubo un comando valido
static bool checkUartCmd(){}

//llama al modulo de timer por sw y checkea el timer del sensor
static bool checkSensorSamplingTimer(){}

//llama al modulo de gpios y pregunta si la tecla fue pulsada
static bool checkButtonPressed(){}

// llama al mudulo del sensor y le pide los datos raw
static bool getCurrentAccelerationFromSensor(aceleration_t* pacel){}

// convierte los datos
static angle_t convertAccelerationToAngle(aceleration_t* pacel){
	angle_t sng;
	return sng;
}

// deberian ser de un modulo screen
static bool updateAnalogScreen(angle_t angle){}

// deberian ser de un modulo screen
static bool updateDigitalScreen(angle_t angle){}


static bool digital_angle_meter_init() {
	// inicializa los modulos perifericos necesarios para que funcione la FSM
	displayMode = DIGITAL;
	return true;
}

void Digital_Angle_Meter_FSM_Init() {
	digitalAngleMeterFsmState = INIT;
}

void Digital_Angle_Meter_FSM_Update() {
	angle_t ang;
	aceleration_t acel;
	bool ret;
	// maneja el delay
	//IF DELAY VENCIDO {}

	// maneja las transiciones de estados
	switch(digitalAngleMeterFsmState) {
		case INIT: {
			if(digital_angle_meter_init()){
				digitalAngleMeterFsmState = ERROR;
			} else {
				digitalAngleMeterFsmState = IDLE;
			}
			break;
		}

		case IDLE: {
			if(checkSensorSamplingTimer()) {
				digitalAngleMeterFsmState = READ_SENSOR;
			} else if(checkUartCmd()) {
				digitalAngleMeterFsmState = HANDLE_UART;
			} else if(checkButtonPressed()) {
				digitalAngleMeterFsmState = HANDLE_BUTTON;
			}
			break;
		}

		case HANDLE_UART: {
			//To be Develop
			break;
		}

		case HANDLE_BUTTON: {
			if(displayMode == DIGITAL) {
				displayMode = ANALOGIC;
			} else {
				displayMode = DIGITAL;
			}
			digitalAngleMeterFsmState = UPDATE_DISPLAY;
			break;
		}

		case READ_SENSOR: {
			// Sampleo cada 10mS
			if(getCurrentAccelerationFromSensor(&acel)){
				digitalAngleMeterFsmState = PROCESS_DATA;
			} else {
				digitalAngleMeterFsmState = ERROR;
			}
			break;
		}

		case PROCESS_DATA: {
			ang = convertAccelerationToAngle(&acel);
			digitalAngleMeterFsmState = UPDATE_DISPLAY;
			break;
		}

		case UPDATE_DISPLAY: {
			if(displayMode == DIGITAL) {
				ret = updateDigitalScreen(ang);
			} else {
				ret = updateAnalogScreen(ang);
			}

			if(ret) {
				digitalAngleMeterFsmState = IDLE;
			} else {
				digitalAngleMeterFsmState = ERROR;
			}

			break;
		}

		case ERROR: {
			//CHANGE LED BLINK FREQ
			//CHECK PERIPHERAL INTEGRITY
			//FEEDBACK BY UART
			break;
		}

		default: {
			digitalAngleMeterFsmState = INIT;
			break;
		}
	}
}
