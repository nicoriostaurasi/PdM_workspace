/*
 * API_digital_angle_meter.c
 *
 *  Created on: 11 abr 2026
 *      Author: nicol
 */

#include "API_digital_angle_meter.h"
#include <stdbool.h>
#include <math.h>
#include <stdio.h>
#include "API_debounce.h"
#include "API_delay.h"
#include "API_i2c.h"
#include "API_uart.h"
#include "API_gpios.h"
#include "API_adxl345.h"
#include "API_ssd1306.h"
#include "API_screen.h"
#include "API_accelerometer.h"
#include "API_angle.h"
#include "API_graphic.h"
#include "API_cmdparser.h"

#define FSM_TICK_DELAY 1
#define HEARTBEAT_RATE 20
#define HEARTBEAT_RATE_FAIL 100
#define SENSOR_SAMPLE_RATE 10
#define SENSOR_READ_MAX_RETRIES 3

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
static displayMode_t displayMode = ANALOGIC;
static delay_t fsmDelay;
static delay_t heartBeatLedTimer;
static delay_t sampleRateTimer;

static void printCurrentAngle(angles_t currentAngle){
	static uint8_t currentAngleBuffer[128];
	sprintf((char*)currentAngleBuffer,"\nthe current angle is:\r\nPITCH: %.2f°	ROLL: %.2f°\r\n",currentAngle.pitch,currentAngle.roll);
	uartSendString(currentAngleBuffer);
}

static void printCurrentAcceleration(ADXL345_AccelG_t currentAccel){
	static uint8_t currentAccelerationBuffer[128];
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
		uartSendString((uint8_t*)"\nthe current mode is DIGITAL\r\n");
	} else {
		uartSendString((uint8_t*)"\nthe current mode is ANALOGIC\r\n");
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

static void getLoopbackState(void){
	if(uartGetLoopback()){
		uartSendString((uint8_t*)"\nUART loopback is ON\r\n");
	} else {
		uartSendString((uint8_t*)"\nUART loopback is OFF\r\n");
	}
}

static void enableLoopback(void){
	uartSetLoopback(true);
	uartSendString((uint8_t*)"\nUART loopback enabled\r\n");
}

static void disableLoopback(void){
	uartSetLoopback(false);
	uartSendString((uint8_t*)"\nUART loopback disabled\r\n");
}

static void printSystemStatus(void){
	static uint8_t statusBuffer[96];
	bool allOk = true;

	uartSendString((uint8_t*)"\n--- SYSTEM STATUS ---\r\n");

	/* UART: si estamos contestando, anda. Reportamos baudrate y loopback. */
	sprintf((char*)statusBuffer,
			"UART         : OK (baud=%lu, loopback=%s)\r\n",
			getCurrentBaudrate(),
			uartGetLoopback() ? "ON" : "OFF");
	uartSendString(statusBuffer);

	/* ADXL345: WHO_AM_I por I2C, no reconfigura nada */
	if(adxl345_isAlive()){
		uartSendString((uint8_t*)"ADXL345      : OK\r\n");
	} else {
		uartSendString((uint8_t*)"ADXL345      : FAIL\r\n");
		allOk = false;
	}

	/* SSD1306: ping I2C no invasivo */
	if(ssd1306_isAlive()){
		uartSendString((uint8_t*)"OLED SSD1306 : OK\r\n");
	} else {
		uartSendString((uint8_t*)"OLED SSD1306 : FAIL\r\n");
		allOk = false;
	}

	if(allOk){
		uartSendString((uint8_t*)"Overall      : HEALTHY\r\n");
	} else {
		uartSendString((uint8_t*)"Overall      : DEGRADED\r\n");
	}
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
	bool allOk = true;

	if(!init_i2c_1()){
		uartSendString((uint8_t*)"\nI2C BUS, is not working...\r\n");
		allOk = false;
	}

	if(!accelerometer_initSensor()){
		uartSendString((uint8_t*)"\nAccelerometer Sensor, is not working...\r\n");
		allOk = false;
	}

	if(!screen_start()){
		uartSendString((uint8_t*)"\nOLED Screen, is not working...\r\n");
		allOk = false;
	}

	if(allOk){
		uartSendString((uint8_t*)"\nSystem is not failing, recovering OK...\r\n");
		digitalAngleMeterFsmState = INIT;
	}
	HAL_Delay(1000);

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
 				        	printSystemStatus();
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

 				        case CMD_LOOPBACK_GET:{
 				        	getLoopbackState();
 				        	break;
 				        }

 				        case CMD_LOOPBACK_ON:{
 				        	enableLoopback();
 				        	break;
 				        }

 				        case CMD_LOOPBACK_OFF:{
 				        	disableLoopback();
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
				static uint8_t readSensorRetries = 0;
				if(getCurrentAccelerationFromSensor(&currentAccel)){
					readSensorRetries = 0;
					digitalAngleMeterFsmState = PROCESS_DATA;
				} else {
					readSensorRetries++;
					if(readSensorRetries >= SENSOR_READ_MAX_RETRIES){
						readSensorRetries = 0;
						delayWrite(&heartBeatLedTimer,HEARTBEAT_RATE_FAIL);
						digitalAngleMeterFsmState = ERROR_STATE;
					} else {
						// reintento en el próximo tick sin bajar a ERROR_STATE
						digitalAngleMeterFsmState = IDLE;
					}
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
