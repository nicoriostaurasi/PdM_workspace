/*
 * API_ssd1306.c
 *
 *  Created on: 12 abr 2026
 *      Author: nicol
 */
#include "API_ssd1306.h"
#include "API_i2c.h"
#include <string.h>


#define SSD1306_I2C_ADDR 0x78
#define SSD1306_I2C_TIMEOUT 1000

#define SSD1306_DEACTIVATE_SCROLL 0x2E


static bool ssd1306_writeReg(uint8_t reg, uint8_t value){
	return i2c_memWrite(SSD1306_I2C_ADDR, reg, &value, 1, SSD1306_I2C_TIMEOUT);
}

static bool ssd1306_writeMultiReg(uint8_t reg, uint8_t *value, uint16_t size){
	return i2c_memWrite(SSD1306_I2C_ADDR, reg, value, size, SSD1306_I2C_TIMEOUT);
}

static bool ssd1306_sendCommand(uint8_t command){
	return ssd1306_writeReg(0x00, command);
}

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

    ssd1306_fill(COLOR_OFF);

    return true;
}

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

