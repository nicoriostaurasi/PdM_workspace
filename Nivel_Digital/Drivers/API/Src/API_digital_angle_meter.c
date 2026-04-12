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

#define FSM_TICK_DELAY 1
#define HEARTBEAT_RATE 50
#define SENSOR_SAMPLE_RATE 10
#define PI 3.14159265358979323846f



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

typedef struct
{
    float pitch;
    float roll;
} angles_t;

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
	return adxl345_readGAccel(pAccel);
}

static float rad_to_deg(float rad){
    return rad * (180.0f / PI);
}


// convierte los datos
static angles_t convertAccelerationToAngle(ADXL345_AccelG_t* pAccel){
	angles_t currentAngle;
	currentAngle.pitch = -1.0;
	currentAngle.roll = -1.0;

	if(pAccel==NULL){
		return currentAngle;
	}

	currentAngle.pitch = rad_to_deg(
        atan2f(pAccel->x, sqrtf((pAccel->y * pAccel->y) + (pAccel->z * pAccel->z)))
    );

	currentAngle.roll = rad_to_deg(
        atan2f(pAccel->y, sqrtf((pAccel->x * pAccel->x) + (pAccel->z * pAccel->z)))
    );

	return currentAngle;
}

// deberian ser de un modulo screen
static bool updateAnalogScreen(angles_t angle){
	ssd1306_fill(COLOR_OFF);
	ssd1306_gotoXY(0,0);
	ssd1306_puts(" TBD", &Font_11x18, 1);
	return ssd1306_updateScreen();
}

static void floatToString2Dec(float value, char *out)
{
    int32_t int_part;
    int32_t dec_part;
    int idx = 0;

    if (value < 0.0f)
    {
        out[idx++] = '-';
        value = -value;
    }

    int_part = (int32_t)value;
    dec_part = (int32_t)((value - (float)int_part) * 100.0f + 0.5f);

    if (dec_part >= 100)
    {
        int_part += 1;
        dec_part = 0;
    }

    /* convertir parte entera */
    if (int_part >= 100)
    {
        out[idx++] = (char)('0' + (int_part / 100) % 10);
        out[idx++] = (char)('0' + (int_part / 10) % 10);
        out[idx++] = (char)('0' + (int_part % 10));
    }
    else if (int_part >= 10)
    {
        out[idx++] = (char)('0' + (int_part / 10) % 10);
        out[idx++] = (char)('0' + (int_part % 10));
    }
    else
    {
        out[idx++] = (char)('0' + int_part);
    }

    out[idx++] = '.';
    out[idx++] = (char)('0' + (dec_part / 10) % 10);
    out[idx++] = (char)('0' + (dec_part % 10));
    out[idx] = '\0';
}

static void displayPitchRollDigital(float pitch, float roll)
{
    char spitch[12];
    char sroll[12];

    floatToString2Dec(pitch, spitch);
    floatToString2Dec(roll, sroll);

    ssd1306_fill(COLOR_OFF);

    ssd1306_gotoXY(8, 0);
    ssd1306_puts("PITCH", &Font_7x10, 1);

    ssd1306_gotoXY(72, 0);
    ssd1306_puts("ROLL", &Font_7x10, 1);

    ssd1306_gotoXY(0, 18);
    ssd1306_puts(spitch, &Font_11x18, 1);

    ssd1306_gotoXY(64, 18);
    ssd1306_puts(sroll, &Font_11x18, 1);

}

// deberian ser de un modulo screen
static bool updateDigitalScreen(angles_t angle){
    ssd1306_fill(COLOR_ON);
	displayPitchRollDigital(angle.pitch,angle.roll);
	return ssd1306_updateScreen();
}

static void checkHeartBeatTimer(void) {
	if(delayRead(&heartBeatLedTimer)){
		board_toggle_led();
	}
}

static bool digital_angle_meter_init() {
	// inicializa los modulos perifericos necesarios para que funcione la FSM
	displayMode = DIGITAL;

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

	if(!adxl345_init()){
		return false;
	}

	if(!ssd1306_init()){
		return false;
	}

	ssd1306_puts(" NGRT CESE", &Font_11x18, 1);
	ssd1306_gotoXY(0,20);
	ssd1306_puts("   FIUBA", &Font_11x18, 1);


	ssd1306_updateScreen();

	// Para mostrar la pantalla de inicio
	HAL_Delay(1000);

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
