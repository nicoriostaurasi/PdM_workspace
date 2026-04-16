/**
 * @file API_adxl345.c
 * @brief Driver de bajo nivel para el acelerometro ADXL345 via I2C.
 * @date 12 abr 2026
 * @author Nicolas Rios Taurasi
 */

#include <stdlib.h>

#include "API_adxl345.h"
#include "API_i2c.h"

#define ADXL345_I2C_ADDRESS   (0x1D << 1)

#define ADXL345_REG_DEVID         0x00
#define ADXL345_REG_BW_RATE       0x2C
#define ADXL345_REG_POWER_CTL     0x2D
#define ADXL345_REG_DATA_FORMAT   0x31
#define ADXL345_REG_DATA_BASE     0x32
#define BYTE_SHIFT 				  0x08
#define ADXL345_DEVID_VALUE       0xE5

#define ADXL345_I2C_TIMEOUT 	  1000

/**
 * @brief Lee un unico registro del ADXL345.
 * @param reg Direccion del registro a leer.
 * @param value Puntero donde se almacena el byte leido.
 * @return true si la lectura fue exitosa, false en caso contrario.
 */
static bool adxl345_readReg(uint8_t reg, uint8_t *value){
	return i2c_memRead(ADXL345_I2C_ADDRESS, reg, value, 1, ADXL345_I2C_TIMEOUT);
}

/**
 * @brief Lee multiples registros consecutivos del ADXL345.
 * @param reg Direccion del primer registro a leer.
 * @param buffer Puntero al buffer donde se almacenan los bytes leidos.
 * @param len Cantidad de bytes a leer.
 * @return true si la lectura fue exitosa, false en caso contrario.
 */
static bool adxl345_readRegs(uint8_t reg, uint8_t *buffer, uint16_t len){
	return i2c_memRead(ADXL345_I2C_ADDRESS, reg, buffer, len, ADXL345_I2C_TIMEOUT);
}

/**
 * @brief Escribe un unico registro del ADXL345.
 * @param reg Direccion del registro a escribir.
 * @param value Valor a escribir en el registro.
 * @return true si la escritura fue exitosa, false en caso contrario.
 */
static bool adxl345_writeReg(uint8_t reg, uint8_t value){
	return i2c_memWrite(ADXL345_I2C_ADDRESS, reg, &value, 1, ADXL345_I2C_TIMEOUT);
}

/**
 * @brief Inicializa el acelerometro ADXL345.
 *
 * Verifica la presencia del dispositivo en el bus I2C, comprueba que el
 * DEVID sea 0xE5 y configura: data rate 100 Hz, rango +/-2 g con FULL_RES
 * habilitado, y activa el modo de medicion.
 *
 * @return true si la inicializacion completa fue exitosa, false en caso contrario.
 */
bool adxl345_init(void){
    bool ret;
    uint8_t devId = 0;

    ret = i2c_isDeviceReady(ADXL345_I2C_ADDRESS, 2, ADXL345_I2C_TIMEOUT);
    if (!ret){
        return false;
    }

    ret = adxl345_readDeviceId(&devId);
    if (!ret){
        return false;
    }

    if (devId != ADXL345_DEVID_VALUE){
        return false;
    }

    /* Standby antes de configurar */
    ret = adxl345_writeReg(ADXL345_REG_POWER_CTL, 0x00);
    if (!ret){
    	return false;
    }

    /* Data rate = 100 Hz */
    ret = adxl345_writeReg(ADXL345_REG_BW_RATE, 0x0A);
    if (!ret){
    	return false;
    }

    /* FULL_RES = 1, rango ±2g */
    ret = adxl345_writeReg(ADXL345_REG_DATA_FORMAT, 0x08);
    if (!ret){
    	return false;
    }

    /* Measurement mode */
    ret = adxl345_writeReg(ADXL345_REG_POWER_CTL, 0x08);
    if (!ret){
    	return false;
    }

	return true;
}

/**
 * @brief Lee el registro DEVID del ADXL345.
 * @param devid Puntero donde se almacena el valor del DEVID.
 * @return true si la lectura fue exitosa, false en caso contrario.
 */
bool adxl345_readDeviceId(uint8_t *devid){
    return adxl345_readReg(ADXL345_REG_DEVID, devid);
}

/**
 * @brief Verifica si el ADXL345 responde correctamente.
 *
 * Lee el DEVID y compara con el valor esperado (0xE5).
 *
 * @return true si el dispositivo responde y el DEVID es valido, false en caso contrario.
 */
bool adxl345_isAlive(void){
    uint8_t devId = 0;
    if (!adxl345_readDeviceId(&devId)){
        return false;
    }
    return (devId == ADXL345_DEVID_VALUE);
}

/**
 * @brief Lee los 6 registros de datos crudos de aceleracion (X, Y, Z).
 *
 * Los datos se ensamblan desde los registros DATA_X0..DATA_Z1 en formato
 * little-endian de 16 bits con signo.
 *
 * @param raw Puntero a la estructura donde se almacenan los valores crudos.
 * @return true si la lectura fue exitosa, false si el puntero es NULL o fallo la lectura.
 */
bool adxl345_readRawAcceleration(adxl345_raw_t *raw)
{
	bool ret;
    uint8_t rawData[6];

    if (raw == NULL){
        return false;
    }

    ret = adxl345_readRegs(ADXL345_REG_DATA_BASE, rawData, 6);

    if (!ret){
        return false;
    }

    raw->x = (int16_t)((rawData[1] << BYTE_SHIFT) | rawData[0]);
    raw->y = (int16_t)((rawData[3] << BYTE_SHIFT) | rawData[2]);
    raw->z = (int16_t)((rawData[5] << BYTE_SHIFT) | rawData[4]);

    return true;
}
