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
#include "API_cmdparser.h"
#include <stdio.h>

#define FSM_TICK_DELAY 1
#define HEARTBEAT_RATE 20
#define HEARTBEAT_RATE_FAIL 100
#define SENSOR_SAMPLE_RATE 10

static cmd_id_t currentCmd;

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

static void printCurrentAngle(angles_t currentAngle){
 	uint8_t currentAngleBuffer[128];
	sprintf((char*)currentAngleBuffer,"\nthe current angle is:\r\nPITCH: %.2f°	ROLL: %.2f°\r\n",currentAngle.pitch,currentAngle.roll);
	uartSendString(currentAngleBuffer);
}

static void printCurrentAcceleration(ADXL345_AccelG_t currentAccel){
 	uint8_t currentAccelerationBuffer[128];
	sprintf((char*)currentAccelerationBuffer,"\nthe current acceleration is:\r\nX: %.2f Y: %.2f Z: %.2f\r\n",currentAccel.x,currentAccel.y,currentAccel.z);
	uartSendString(currentAccelerationBuffer);
}

static void toggleDisplayMode(void){
 	uartSendString((uint8_t*)"\ntoggle mode\r\n");
	if(displayMode == DIGITAL) {
		displayMode = ANALOGIC;
	} else {
		displayMode = DIGITAL;
	}
}


static void getDisplayMode(void){
	if(displayMode == DIGITAL) {
		uartSendString((uint8_t*)"\nthe current mode is ANALOGIC\r\n");
	} else {
		uartSendString((uint8_t*)"\nthe current mode is DIGITAL\r\n");
	}
}

static void setDisplayModeToDigital(void){
	displayMode = DIGITAL;
 	uartSendString((uint8_t*)"\nset to digital\r\n");
}

static void setDisplayModeToAnalogic(void){
	displayMode = ANALOGIC;
 	uartSendString((uint8_t*)"\nset to analog\r\n");
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

	HAL_Delay(1000);

	return true;
}

static void checkSystemIntegrity(void){

	bool i2cFail = false;
	bool accelerometerFail = false;
	bool screenFail = false;

	if(!init_i2c_1()){
		uartSendString((uint8_t*)"\nI2C BUS, is not working...\r\n");
		i2cFail = true;
		HAL_Delay(500);
	}

	if(!accelerometer_initSensor()){
		uartSendString((uint8_t*)"\nAccelerometer Sensor, is not working...\r\n");
		accelerometerFail = true;
		HAL_Delay(500);
	}

	if(!screen_start()){
		uartSendString((uint8_t*)"\nOLED Screen, is not working...\r\n");
		screenFail = true;
		HAL_Delay(500);
	}

	if(!i2cFail && !accelerometerFail && !screenFail){
		uartSendString((uint8_t*)"\nSystem is not failing, recovering OK...\r\n");
		digitalAngleMeterFsmState = INIT;
		digital_angle_meter_init();
	}



}


void Digital_Angle_Meter_FSM_Init() {
	digitalAngleMeterFsmState = INIT;
}

void Digital_Angle_Meter_FSM_Update() {
	static angles_t currentAngle;
	static ADXL345_AccelG_t currentAccel;

	bool ret;

	if(delayRead(&fsmDelay)){

		switch(digitalAngleMeterFsmState) {
			case INIT: {
				if(!digital_angle_meter_init()){
					delayWrite(&heartBeatLedTimer,HEARTBEAT_RATE_FAIL);
					digitalAngleMeterFsmState = ERROR_STATE;
				} else {
					digitalAngleMeterFsmState = IDLE;
				}
				break;
			}

			case IDLE: {
				if(cmdGetPendingCommand(&currentCmd)) {
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
				switch (currentCmd) {
 				        case CMD_HELP:{
				            cmdPrintHelp();
				            break;
 				        }

 				        case CMD_READ_ANGLE:{
 				        	printCurrentAngle(currentAngle);
				            break;
 				        }

 				        case CMD_READ_ACCELERATION:{
 				        	printCurrentAcceleration(currentAccel);
 				        	break;
 				        }

 				        case CMD_STATUS:{
 				        	uartSendString((uint8_t*)"\nthe current status is\r\n");
 				        	break;
 				        }

 				        case CMD_MODE_GET:{
 				        	getDisplayMode();
 				        	break;
 				        }

 				        case CMD_MODE_TOGGLE:{
 							toggleDisplayMode();
 				        	break;
 				        }

 				        case CMD_MODE_DIGITAL:{
 				        	setDisplayModeToDigital();
 				        	break;
 				        }

 				        case CMD_MODE_ANALOG:{
 				        	setDisplayModeToAnalogic();
 				        	break;
 				        }
						default:{
 				        	uartSendString((uint8_t*)"\nunknown command\r\n");
							break;
						}
				    }
				    digitalAngleMeterFsmState = IDLE;
				    break;
			}

			case HANDLE_BUTTON: {
				toggleDisplayMode();
				digitalAngleMeterFsmState = UPDATE_DISPLAY;
				break;
			}

			case READ_SENSOR: {
				if(getCurrentAccelerationFromSensor(&currentAccel)){
					digitalAngleMeterFsmState = PROCESS_DATA;
				} else {
					delayWrite(&heartBeatLedTimer,HEARTBEAT_RATE_FAIL);
					digitalAngleMeterFsmState = ERROR_STATE;
				}
				break;
			}

			case PROCESS_DATA: {
				currentAngle = convertAccelerationToAngle(&currentAccel);
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
					delayWrite(&heartBeatLedTimer,HEARTBEAT_RATE_FAIL);
					digitalAngleMeterFsmState = ERROR_STATE;
				}

				break;
			}

			case ERROR_STATE: {
				checkSystemIntegrity();
				checkHeartBeatTimer();
				break;
			}

			default: {
				digitalAngleMeterFsmState = INIT;
				break;
			}
		}
	}
}
