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
	FSM_INIT = 0,
	FSM_IDLE,
	FSM_HANDLE_UART,
	FSM_HANDLE_BUTTON,
	FSM_READ_SENSOR,
	FSM_PROCESS_DATA,
	FSM_UPDATE_DISPLAY,
	FSM_ERROR_STATE,
} digitalAngleMeterState_t;

typedef enum {
	DISPLAY_DIGITAL = 0,
	DISPLAY_ANALOGIC,
} displayMode_t;


static digitalAngleMeterState_t digitalAngleMeterFsmState = FSM_INIT;
static displayMode_t displayMode = DISPLAY_ANALOGIC;
static delay_t fsmDelay;
static delay_t heartBeatLedTimer;
static delay_t sampleRateTimer;

static void printCurrentAngle(angles_t currentAngle){
	static uint8_t currentAngleBuffer[128];
	sprintf((char*)currentAngleBuffer,"\nthe current angle is:\r\nPITCH: %.2f°	ROLL: %.2f°\r\n",currentAngle.pitch,currentAngle.roll);
	uart_sendString(currentAngleBuffer);
}

static void printCurrentAcceleration(adxl345_accelG_t currentAccel){
	static uint8_t currentAccelerationBuffer[128];
	sprintf((char*)currentAccelerationBuffer,"\nthe current acceleration is:\r\nX: %.2f Y: %.2f Z: %.2f\r\n",currentAccel.x,currentAccel.y,currentAccel.z);
	uart_sendString(currentAccelerationBuffer);
}

static void toggleDisplayMode(void){
 	uart_sendString((uint8_t*)"\ntoggle mode\r\n");
	if(displayMode == DISPLAY_DIGITAL) {
		displayMode = DISPLAY_ANALOGIC;
	} else {
		displayMode = DISPLAY_DIGITAL;
	}
}


static void getDisplayMode(void){
	if(displayMode == DISPLAY_DIGITAL) {
		uart_sendString((uint8_t*)"\nthe current mode is DIGITAL\r\n");
	} else {
		uart_sendString((uint8_t*)"\nthe current mode is ANALOGIC\r\n");
	}
}

static void setDisplayModeToDigital(void){
	displayMode = DISPLAY_DIGITAL;
 	uart_sendString((uint8_t*)"\nset to digital\r\n");
}

static void setDisplayModeToAnalogic(void){
	displayMode = DISPLAY_ANALOGIC;
 	uart_sendString((uint8_t*)"\nset to analog\r\n");
}

static void getLoopbackState(void){
	if(uart_getLoopback()){
		uart_sendString((uint8_t*)"\nUART loopback is ON\r\n");
	} else {
		uart_sendString((uint8_t*)"\nUART loopback is OFF\r\n");
	}
}

static void enableLoopback(void){
	uart_setLoopback(true);
	uart_sendString((uint8_t*)"\nUART loopback enabled\r\n");
}

static void disableLoopback(void){
	uart_setLoopback(false);
	uart_sendString((uint8_t*)"\nUART loopback disabled\r\n");
}

static void printSystemStatus(void){
	static uint8_t statusBuffer[96];
	bool allOk = true;

	uart_sendString((uint8_t*)"\n--- SYSTEM STATUS ---\r\n");

	/* UART: si estamos contestando, anda. Reportamos baudrate y loopback. */
	sprintf((char*)statusBuffer,
			"UART         : OK (baud=%lu, loopback=%s)\r\n",
			uart_getCurrentBaudrate(),
			uart_getLoopback() ? "ON" : "OFF");
	uart_sendString(statusBuffer);

	/* ADXL345: WHO_AM_I por I2C, no reconfigura nada */
	if(adxl345_isAlive()){
		uart_sendString((uint8_t*)"ADXL345      : OK\r\n");
	} else {
		uart_sendString((uint8_t*)"ADXL345      : FAIL\r\n");
		allOk = false;
	}

	/* SSD1306: ping I2C no invasivo */
	if(ssd1306_isAlive()){
		uart_sendString((uint8_t*)"OLED SSD1306 : OK\r\n");
	} else {
		uart_sendString((uint8_t*)"OLED SSD1306 : FAIL\r\n");
		allOk = false;
	}

	if(allOk){
		uart_sendString((uint8_t*)"Overall      : HEALTHY\r\n");
	} else {
		uart_sendString((uint8_t*)"Overall      : DEGRADED\r\n");
	}
}


//llama al modulo de timer por sw y checkea el timer del sensor
static bool checkSensorSamplingTimer(){
	return delay_read(&sampleRateTimer);
}

//llama al modulo de gpios y pregunta si la tecla fue pulsada
static bool checkButtonPressed(){
	return debounce_readKey();
}

// llama al mudulo del sensor y le pide los datos raw
static bool getCurrentAccelerationFromSensor(adxl345_accelG_t* pAccel){
	return accelerometer_readAccelerationG(pAccel);
}

static void checkHeartBeatTimer(void) {
	if(delay_read(&heartBeatLedTimer)){
		gpios_toggleLed();
	}
}

static bool digital_angle_meter_init() {
	// inicializa los modulos perifericos necesarios para que funcione la FSM
	if(!i2c_init1()){
		return false;
	}

	if(!uart_init()){
		return false;
	}

	gpios_init();

	delay_write(&fsmDelay,FSM_TICK_DELAY);
	delay_write(&heartBeatLedTimer,HEARTBEAT_RATE);
	delay_write(&sampleRateTimer,SENSOR_SAMPLE_RATE);

	if(!accelerometer_initSensor()){
		return false;
	}

	if(!screen_start()){
		return false;
	}

	graphic_gotoXY(0,10);
	graphic_puts(" NGRT CESE", &Font_11x18, 1);
	graphic_gotoXY(0,30);
	graphic_puts("   FIUBA", &Font_11x18, 1);

	if(!graphic_update()){
		return false;
	}

	HAL_Delay(1000);

	return true;
}

static void checkSystemIntegrity(void){
	bool allOk = true;

	if(!i2c_init1()){
		uart_sendString((uint8_t*)"\nI2C BUS, is not working...\r\n");
		allOk = false;
	}

	if(!accelerometer_initSensor()){
		uart_sendString((uint8_t*)"\nAccelerometer Sensor, is not working...\r\n");
		allOk = false;
	}

	if(!screen_start()){
		uart_sendString((uint8_t*)"\nOLED Screen, is not working...\r\n");
		allOk = false;
	}

	if(allOk){
		uart_sendString((uint8_t*)"\nSystem is not failing, recovering OK...\r\n");
		digitalAngleMeterFsmState = FSM_INIT;
	}
	HAL_Delay(1000);

}


void digitalAngleMeter_fsmInit() {
	digitalAngleMeterFsmState = FSM_INIT;
}

void digitalAngleMeter_fsmUpdate() {
	static angles_t currentAngle;
	static adxl345_accelG_t currentAccel;

	bool ret;

	if(delay_read(&fsmDelay)){

		switch(digitalAngleMeterFsmState) {
			case FSM_INIT: {
				if(!digital_angle_meter_init()){
					delay_write(&heartBeatLedTimer,HEARTBEAT_RATE_FAIL);
					digitalAngleMeterFsmState = FSM_ERROR_STATE;
				} else {
					digitalAngleMeterFsmState = FSM_IDLE;
				}
				break;
			}

			case FSM_IDLE: {
				if(cmd_getPendingCommand(&currentCmd)) {
					digitalAngleMeterFsmState = FSM_HANDLE_UART;
				} else if(checkButtonPressed()) {
					digitalAngleMeterFsmState = FSM_HANDLE_BUTTON;
				}else if(checkSensorSamplingTimer()) {
					digitalAngleMeterFsmState = FSM_READ_SENSOR;
				}

				checkHeartBeatTimer();
				break;
			}

			case FSM_HANDLE_UART: {
				switch (currentCmd) {
 				        case CMD_HELP:{
				            cmd_printHelp();
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
 				        	uart_sendString((uint8_t*)"\nunknown command\r\n");
							break;
						}
				    }
				    digitalAngleMeterFsmState = FSM_IDLE;
				    break;
			}

			case FSM_HANDLE_BUTTON: {
				toggleDisplayMode();
				digitalAngleMeterFsmState = FSM_UPDATE_DISPLAY;
				break;
			}

			case FSM_READ_SENSOR: {
				static uint8_t readSensorRetries = 0;
				if(getCurrentAccelerationFromSensor(&currentAccel)){
					readSensorRetries = 0;
					digitalAngleMeterFsmState = FSM_PROCESS_DATA;
				} else {
					readSensorRetries++;
					if(readSensorRetries >= SENSOR_READ_MAX_RETRIES){
						readSensorRetries = 0;
						delay_write(&heartBeatLedTimer,HEARTBEAT_RATE_FAIL);
						digitalAngleMeterFsmState = FSM_ERROR_STATE;
					} else {
						// reintento en el próximo tick sin bajar a FSM_ERROR_STATE
						digitalAngleMeterFsmState = FSM_IDLE;
					}
				}
				break;
			}

			case FSM_PROCESS_DATA: {
				currentAngle = angle_fromAcceleration(&currentAccel);
				digitalAngleMeterFsmState = FSM_UPDATE_DISPLAY;
				break;
			}

			case FSM_UPDATE_DISPLAY: {
				if(displayMode == DISPLAY_DIGITAL) {
					ret = screen_updateDigital(currentAngle);
				} else {
					ret = screen_updateAnalog(currentAngle);
				}

				if(ret) {
					digitalAngleMeterFsmState = FSM_IDLE;
				} else {
					delay_write(&heartBeatLedTimer,HEARTBEAT_RATE_FAIL);
					digitalAngleMeterFsmState = FSM_ERROR_STATE;
				}

				break;
			}

			case FSM_ERROR_STATE: {
				checkSystemIntegrity();
				checkHeartBeatTimer();
				break;
			}

			default: {
				digitalAngleMeterFsmState = FSM_INIT;
				break;
			}
		}
	}
}
