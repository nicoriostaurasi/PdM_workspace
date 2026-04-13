/*
 * API_digital_angle_meter.c
 *
 *  Created on: 11 abr 2026
 *      Author: nicol
 */

#include "API_digital_angle_meter.h"
#include <stdbool.h>
#include "API_debounce.h"
#include "API_delay.h"
#include "API_i2c.h"
#include "API_uart.h"
#include "API_gpios.h"
#include "API_adxl345.h"
#include "API_ssd1306.h"
#include <math.h>
#include "API_screen.h"
#include "API_accelerometer.h"
#include "API_screen.h"
#include "API_angle.h"
#include "API_graphic.h"

#define FSM_TICK_DELAY 1
#define HEARTBEAT_RATE 50
#define SENSOR_SAMPLE_RATE 10

typedef enum {
	INIT = 0,
	IDLE,
	HANDLE_UART,
	HANDLE_BUTTON,
	READ_SENSOR,
	PROCESS_DATA,
	UPDATE_DISPLAY,
	ERROR_STATE,
} digitalAngleMeterState_t;

typedef enum {
	DIGITAL = 0,
	ANALOGIC,
} displayMode_t;


static digitalAngleMeterState_t digitalAngleMeterFsmState = INIT;
static displayMode_t displayMode = DIGITAL;
static delay_t fsmDelay;
static delay_t heartBeatLedTimer;
static delay_t sampleRateTimer;

//llama al modulo de uart, y pregunta si hubo un comando valido
static bool checkUartCmd(){
	return false;
}

//llama al modulo de timer por sw y checkea el timer del sensor
static bool checkSensorSamplingTimer(){
	return delayRead(&sampleRateTimer);
}

//llama al modulo de gpios y pregunta si la tecla fue pulsada
static bool checkButtonPressed(){
	return readKey();
}

// llama al mudulo del sensor y le pide los datos raw
static bool getCurrentAccelerationFromSensor(ADXL345_AccelG_t* pAccel){
	return accelerometer_readAccelerationG(pAccel);
}

static void checkHeartBeatTimer(void) {
	if(delayRead(&heartBeatLedTimer)){
		board_toggle_led();
	}
}

static bool digital_angle_meter_init() {
	// inicializa los modulos perifericos necesarios para que funcione la FSM
	displayMode = ANALOGIC;

	if(!init_i2c_1()){
		return false;
	}

	if(!uartInit()){
		return false;
	}

	board_gpios_init();

	delayWrite(&fsmDelay,FSM_TICK_DELAY);
	delayWrite(&heartBeatLedTimer,HEARTBEAT_RATE);
	delayWrite(&sampleRateTimer,SENSOR_SAMPLE_RATE);

	if(!accelerometer_initSensor()){
		return false;
	}

	if(!screen_start()){
		return false;
	}

	ssd1306_puts(" NGRT CESE", &Font_11x18, 1);
	ssd1306_gotoXY(0,20);
	ssd1306_puts("   FIUBA", &Font_11x18, 1);

	if(!graphic_update()){
		return false;
	}

//	HAL_Delay(1000);

	return true;
}

void Digital_Angle_Meter_FSM_Init() {
	digitalAngleMeterFsmState = INIT;
}

void Digital_Angle_Meter_FSM_Update() {
	static angles_t currentAngle;
	static ADXL345_AccelG_t accel;

	bool ret;

	if(delayRead(&fsmDelay)){
		// maneja las transiciones de estados
		switch(digitalAngleMeterFsmState) {
			case INIT: {
				if(!digital_angle_meter_init()){
					digitalAngleMeterFsmState = ERROR_STATE;
				} else {
					digitalAngleMeterFsmState = IDLE;
				}
				break;
			}

			case IDLE: {
				if(checkUartCmd()) {
					digitalAngleMeterFsmState = HANDLE_UART;
				} else if(checkButtonPressed()) {
					digitalAngleMeterFsmState = HANDLE_BUTTON;
				}else if(checkSensorSamplingTimer()) {
					digitalAngleMeterFsmState = READ_SENSOR;
				}

				checkHeartBeatTimer();
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
				if(getCurrentAccelerationFromSensor(&accel)){
					digitalAngleMeterFsmState = PROCESS_DATA;
				} else {
					digitalAngleMeterFsmState = ERROR_STATE;
				}
				break;
			}

			case PROCESS_DATA: {
				currentAngle = convertAccelerationToAngle(&accel);
				digitalAngleMeterFsmState = UPDATE_DISPLAY;
				break;
			}

			case UPDATE_DISPLAY: {
				if(displayMode == DIGITAL) {
					ret = updateDigitalScreen(currentAngle);
				} else {
					ret = updateAnalogScreen(currentAngle);
				}

				if(ret) {
					digitalAngleMeterFsmState = IDLE;
				} else {
					digitalAngleMeterFsmState = ERROR_STATE;
				}

				break;
			}

			case ERROR_STATE: {
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
}
