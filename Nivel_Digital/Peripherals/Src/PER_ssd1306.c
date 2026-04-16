/**
 * @file  PER_ssd1306.c
 * @brief Driver para la pantalla OLED SSD1306 via I2C.
 * @date 12 abr 2026
 * @author Ing. Nicolás Gabriel Rios Taurasi
 */

#include <string.h>

#include "PER_ssd1306.h"
#include "BSP_i2c.h"

#define SSD1306_I2C_ADDR 0x78
#define SSD1306_I2C_TIMEOUT 1000

#define SSD1306_DEACTIVATE_SCROLL 0x2E

/**
 * @brief Escribe un byte en un registro del SSD1306.
 * @param reg Direccion del registro (byte de control).
 * @param value Valor a escribir.
 * @return true si la escritura fue exitosa, false en caso contrario.
 */
static bool ssd1306_writeReg(uint8_t reg, uint8_t value){
	return i2c_memWrite(SSD1306_I2C_ADDR, reg, &value, 1, SSD1306_I2C_TIMEOUT);
}

/**
 * @brief Escribe multiples bytes en un registro del SSD1306.
 * @param reg Direccion del registro (byte de control).
 * @param value Puntero al buffer con los datos a enviar.
 * @param size Cantidad de bytes a escribir.
 * @return true si la escritura fue exitosa, false en caso contrario.
 */
static bool ssd1306_writeMultiReg(uint8_t reg, uint8_t *value, uint16_t size){
	return i2c_memWrite(SSD1306_I2C_ADDR, reg, value, size, SSD1306_I2C_TIMEOUT);
}

/**
 * @brief Envia un comando al SSD1306 (registro de control 0x00).
 * @param command Byte de comando a enviar.
 * @return true si el envio fue exitoso, false en caso contrario.
 */
static bool ssd1306_sendCommand(uint8_t command){
	return ssd1306_writeReg(0x00, command);
}

/**
 * @brief Verifica si el SSD1306 responde en el bus I2C.
 * @return true si el dispositivo esta listo, false en caso contrario.
 */
bool ssd1306_isAlive(void){
    return i2c_isDeviceReady(SSD1306_I2C_ADDR, 1, SSD1306_I2C_TIMEOUT);
}

/**
 * @brief Inicializa la pantalla OLED SSD1306.
 *
 * Verifica la presencia del dispositivo, envia la secuencia completa de
 * configuracion (modo de direccionamiento por pagina, contraste, multiplex,
 * oscilador, pre-carga, DC-DC, etc.) y desactiva el scroll.
 *
 * @return true si la inicializacion fue exitosa, false en caso contrario.
 */
bool ssd1306_init(void){
	bool ret;
	ret = i2c_isDeviceReady(SSD1306_I2C_ADDR, 1, SSD1306_I2C_TIMEOUT);
    if (!ret){
        return false;
    }

    // Tomado como referencia
    ssd1306_sendCommand(0xAE); //display off
    ssd1306_sendCommand(0x20); //Set Memory Addressing Mode
    ssd1306_sendCommand(0x10); //00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
    ssd1306_sendCommand(0xB0); //Set Page Start Address for Page Addressing Mode,0-7
    ssd1306_sendCommand(0xC8); //Set COM Output Scan Direction
    ssd1306_sendCommand(0x00); //---set low column address
    ssd1306_sendCommand(0x10); //---set high column address
    ssd1306_sendCommand(0x40); //--set start line address
    ssd1306_sendCommand(0x81); //--set contrast control register
    ssd1306_sendCommand(0xFF);
    ssd1306_sendCommand(0xA1); //--set segment re-map 0 to 127
    ssd1306_sendCommand(0xA6); //--set normal display
    ssd1306_sendCommand(0xA8); //--set multiplex ratio(1 to 64)
    ssd1306_sendCommand(0x3F); //
    ssd1306_sendCommand(0xA4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content
    ssd1306_sendCommand(0xD3); //-set display offset
    ssd1306_sendCommand(0x00); //-not offset
    ssd1306_sendCommand(0xD5); //--set display clock divide ratio/oscillator frequency
    ssd1306_sendCommand(0xF0); //--set divide ratio
    ssd1306_sendCommand(0xD9); //--set pre-charge period
    ssd1306_sendCommand(0x22); //
    ssd1306_sendCommand(0xDA); //--set com pins hardware configuration
    ssd1306_sendCommand(0x12);
    ssd1306_sendCommand(0xDB); //--set vcomh
    ssd1306_sendCommand(0x20); //0x20,0.77xVcc
    ssd1306_sendCommand(0x8D); //--set DC-DC enable
    ssd1306_sendCommand(0x14); //
    ret = ssd1306_sendCommand(0xAF); //--turn on SSD1306 panel

    if (!ret){
        return false;
    }

    ssd1306_sendCommand(SSD1306_DEACTIVATE_SCROLL);

    return true;
}

/**
 * @brief Actualiza la pantalla enviando el framebuffer completo pagina por pagina.
 *
 * Recorre las 8 paginas del display y envia la mitad del ancho por transferencia,
 * dividiendo cada pagina en dos escrituras consecutivas.
 *
 * @param disp Puntero a la estructura display_t que contiene el framebuffer y dimensiones.
 * @return true si la actualizacion fue exitosa, false si disp es NULL o fallo alguna escritura.
 */
bool ssd1306_updateScreen(display_t *disp) {
	uint8_t m;

	bool ret;

	if( disp == NULL ){
			return false;
	}

	for (m = 0; m < 8; m++) {
		ssd1306_sendCommand(0xB0 + m);
		ssd1306_sendCommand(0x00);
		ssd1306_sendCommand(0x10);

    	ret = ssd1306_writeMultiReg(0x40, &disp->buffer[(disp->width) * m], (disp->width)/2);
    	if(!ret){
    		return false;
    	}

    	ssd1306_sendCommand(0xB0 + m);
		ssd1306_sendCommand(0x00);
		ssd1306_sendCommand(0x14);

		ret = ssd1306_writeMultiReg(0x40, &disp->buffer[(disp->width) * m + (disp->width)/2], (disp->width)/2);
		if(!ret){
        	return false;
        }
	}
	return true;
}
