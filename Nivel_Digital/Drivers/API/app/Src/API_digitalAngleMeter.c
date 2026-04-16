/** @file API_digitalAngleMeter.c
 *  @brief Máquina de estados principal del inclinómetro digital.
 *
 *  Implementa la FSM que coordina la lectura del acelerómetro ADXL345,
 *  el cálculo de ángulos (pitch/roll), la actualización de la pantalla
 *  OLED SSD1306, la atención de comandos UART y la pulsación de botón.
 *  Gestiona los modos de visualización (digital / analógico) y el
 *  monitoreo de integridad del sistema.
 *
 *  @date 11 abr 2026
 *  @author Nicolás Rios Taurasi
 */

#include <stdbool.h>
#include <stdio.h>

#include "API_digitalAngleMeter.h"
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
#include "API_cmdParser.h"

/** @brief Período del tick de la FSM principal (en ms) */
#define FSM_TICK_DELAY 1

/** @brief Período del parpadeo del LED heartbeat en funcionamiento normal (en ms) */
#define HEARTBEAT_RATE 20

/** @brief Período del parpadeo del LED heartbeat en estado de error (en ms) */
#define HEARTBEAT_RATE_FAIL 100

/** @brief Período de muestreo del sensor acelerómetro (en ms) */
#define SENSOR_SAMPLE_RATE 10

/** @brief Cantidad máxima de reintentos de lectura del sensor antes de ir a error */
#define SENSOR_READ_MAX_RETRIES 3

/** @brief Último comando UART recibido pendiente de ejecución */
static cmd_id_t currentCmd;

/** @brief Estados de la máquina de estados principal del inclinómetro */
typedef enum {
	FSM_INIT = 0,         /**< Inicialización de todos los módulos periféricos */
	FSM_IDLE,             /**< Espera de eventos (comando UART, botón o timer del sensor) */
	FSM_HANDLE_UART,      /**< Procesamiento del comando UART recibido */
	FSM_HANDLE_BUTTON,    /**< Procesamiento de la pulsación de botón */
	FSM_READ_SENSOR,      /**< Lectura de aceleración desde el ADXL345 */
	FSM_PROCESS_DATA,     /**< Cálculo de ángulos a partir de la aceleración */
	FSM_UPDATE_DISPLAY,   /**< Actualización de la pantalla OLED */
	FSM_ERROR_STATE,      /**< Estado de error con verificación de integridad */
} digitalAngleMeterState_t;

/** @brief Modos de visualización de la pantalla OLED */
typedef enum {
	DISPLAY_DIGITAL = 0,  /**< Visualización numérica de los ángulos */
	DISPLAY_ANALOGIC,     /**< Visualización gráfica (analógica) de los ángulos */
} displayMode_t;


/** @brief Estado actual de la FSM principal */
static digitalAngleMeterState_t digitalAngleMeterFsmState = FSM_INIT;

/** @brief Modo de visualización actual de la pantalla */
static displayMode_t displayMode = DISPLAY_ANALOGIC;

/** @brief Timer por software para el tick de la FSM */
static delay_t fsmDelay;

/** @brief Timer por software para el parpadeo del LED heartbeat */
static delay_t heartBeatLedTimer;

/** @brief Timer por software para el muestreo del sensor */
static delay_t sampleRateTimer;

/**
 * @brief Imprime el ángulo actual (pitch y roll) por UART.
 *
 * @param currentAngle Estructura con los ángulos pitch y roll en grados
 */
static void printCurrentAngle(angles_t currentAngle){
	static uint8_t currentAngleBuffer[128];
	sprintf((char*)currentAngleBuffer,"\nthe current angle is:\r\nPITCH: %.2f°	ROLL: %.2f°\r\n",currentAngle.pitch,currentAngle.roll);
	uart_sendString(currentAngleBuffer);
}

/**
 * @brief Imprime la aceleración actual (X, Y, Z) por UART.
 *
 * @param currentAccel Estructura con las componentes de aceleración en g
 */
static void printCurrentAcceleration(adxl345_accelG_t currentAccel){
	static uint8_t currentAccelerationBuffer[128];
	sprintf((char*)currentAccelerationBuffer,"\nthe current acceleration is:\r\nX: %.2f Y: %.2f Z: %.2f\r\n",currentAccel.x,currentAccel.y,currentAccel.z);
	uart_sendString(currentAccelerationBuffer);
}

/**
 * @brief Alterna el modo de visualización entre digital y analógico.
 *
 * Envía un mensaje informativo por UART e invierte el modo actual.
 */
static void toggleDisplayMode(void){
 	uart_sendString((uint8_t*)"\ntoggle mode\r\n");
	if(displayMode == DISPLAY_DIGITAL) {
		displayMode = DISPLAY_ANALOGIC;
	} else {
		displayMode = DISPLAY_DIGITAL;
	}
}


/**
 * @brief Informa por UART el modo de visualización actual (DIGITAL o ANALOGIC).
 */
static void getDisplayMode(void){
	if(displayMode == DISPLAY_DIGITAL) {
		uart_sendString((uint8_t*)"\nthe current mode is DIGITAL\r\n");
	} else {
		uart_sendString((uint8_t*)"\nthe current mode is ANALOGIC\r\n");
	}
}

/**
 * @brief Fuerza el modo de visualización a DIGITAL e informa por UART.
 */
static void setDisplayModeToDigital(void){
	displayMode = DISPLAY_DIGITAL;
 	uart_sendString((uint8_t*)"\nset to digital\r\n");
}

/**
 * @brief Fuerza el modo de visualización a ANALOGIC e informa por UART.
 */
static void setDisplayModeToAnalogic(void){
	displayMode = DISPLAY_ANALOGIC;
 	uart_sendString((uint8_t*)"\nset to analog\r\n");
}

/**
 * @brief Consulta e informa por UART el estado del loopback de la UART.
 */
static void getLoopbackState(void){
	if(uart_getLoopback()){
		uart_sendString((uint8_t*)"\nUART loopback is ON\r\n");
	} else {
		uart_sendString((uint8_t*)"\nUART loopback is OFF\r\n");
	}
}

/**
 * @brief Habilita el loopback de la UART e informa por UART.
 */
static void enableLoopback(void){
	uart_setLoopback(true);
	uart_sendString((uint8_t*)"\nUART loopback enabled\r\n");
}

/**
 * @brief Deshabilita el loopback de la UART e informa por UART.
 */
static void disableLoopback(void){
	uart_setLoopback(false);
	uart_sendString((uint8_t*)"\nUART loopback disabled\r\n");
}

/**
 * @brief Imprime por UART el estado completo del sistema.
 *
 * Reporta el estado de la UART (baudrate, loopback), del ADXL345 y del
 * SSD1306, y una evaluación global (HEALTHY o DEGRADED).
 */
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


/**
 * @brief Verifica si venció el timer de muestreo del sensor.
 *
 * @return true si el timer expiró y es momento de leer el sensor, false en caso contrario
 */
static bool checkSensorSamplingTimer(){
	return delay_read(&sampleRateTimer);
}

/**
 * @brief Verifica si se detectó una pulsación del botón de usuario.
 *
 * @return true si el botón fue presionado (con antirrebote), false en caso contrario
 */
static bool checkButtonPressed(){
	return debounce_readKey();
}

/**
 * @brief Obtiene la aceleración actual desde el sensor ADXL345.
 *
 * @param pAccel Puntero a la estructura donde se almacenan las componentes de aceleración
 * @return true si la lectura fue exitosa, false si hubo un error de comunicación
 */
static bool getCurrentAccelerationFromSensor(adxl345_accelG_t* pAccel){
	return accelerometer_readAccelerationG(pAccel);
}

/**
 * @brief Verifica el timer del LED heartbeat y alterna el LED si expiró.
 */
static void checkHeartBeatTimer(void) {
	if(delay_read(&heartBeatLedTimer)){
		gpios_toggleLed();
	}
}

/**
 * @brief Inicializa todos los módulos periféricos necesarios para la FSM.
 *
 * Configura I2C, UART, GPIOs, timers por software, acelerómetro, pantalla
 * OLED y muestra la pantalla de bienvenida.
 *
 * @return true si todos los módulos se inicializaron correctamente, false ante cualquier fallo
 */
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

/**
 * @brief Verifica la integridad de los periféricos críticos del sistema.
 *
 * Intenta reinicializar I2C, acelerómetro y pantalla OLED. Si todos
 * responden correctamente, retorna la FSM al estado FSM_INIT para
 * recuperar el funcionamiento normal.
 */
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


/**
 * @brief Inicializa la FSM del inclinómetro digital al estado FSM_INIT.
 *
 * Debe llamarse una vez antes de comenzar a invocar digitalAngleMeter_fsmUpdate.
 */
void digitalAngleMeter_fsmInit() {
	digitalAngleMeterFsmState = FSM_INIT;
}

/**
 * @brief Actualiza la FSM principal del inclinómetro digital.
 *
 * Debe llamarse periódicamente desde el bucle principal (super-loop).
 * Gestiona las transiciones entre los estados de inicialización, espera,
 * atención de comandos UART, pulsación de botón, lectura del sensor,
 * procesamiento de datos, actualización de pantalla y manejo de errores.
 */
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
				            cmdParser_printHelp();
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
