/*
 * API_ssd1306.c
 *
 *  Created on: 12 abr 2026
 *      Author: nicol
 */
#include "API_ssd1306.h"
#include "API_i2c.h"
#include <string.h>

#define SSD1306_WIDTH            128
#define SSD1306_HEIGHT           64

static uint8_t SSD1306_Buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];

typedef struct {
	uint16_t currentX;
	uint16_t currentY;
	uint8_t initialized;
} SSD1306_t;

static SSD1306_t SSD1306;

#define SSD1306_I2C_ADDR 0x78
#define SSD1306_I2C_TIMEOUT 1000

#define SSD1306_WRITECOMMAND(command) ssd1306_writeReg(0x00, (command))


#define SSD1306_RIGHT_HORIZONTAL_SCROLL              0x26
#define SSD1306_LEFT_HORIZONTAL_SCROLL               0x27
#define SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 0x29
#define SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL  0x2A
#define SSD1306_DEACTIVATE_SCROLL                    0x2E
#define SSD1306_ACTIVATE_SCROLL                      0x2F
#define SSD1306_SET_VERTICAL_SCROLL_AREA             0xA3



bool ssd1306_sendCommand(uint8_t command){
	return ssd1306_writeReg(0x00, command);
}

bool ssd1306_writeReg(uint8_t reg, uint8_t value){
	return i2c_memWrite(SSD1306_I2C_ADDR, reg, &value, 1, SSD1306_I2C_TIMEOUT);
}

bool ssd1306_writeMultiReg(uint8_t reg, uint8_t *value, uint16_t size){
	return i2c_memWrite(SSD1306_I2C_ADDR, reg, value, size, SSD1306_I2C_TIMEOUT);
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

    ssd1306_updateScreen();

    SSD1306.currentX = 0;
    SSD1306.currentY = 0;

    SSD1306.initialized = 1;

    return true;
}

void ssd1306_fill(uint8_t value) {
	/* Set memory */
	memset(SSD1306_Buffer, value , sizeof(SSD1306_Buffer));
}

void ssd1306_gotoXY(uint16_t x, uint16_t y) {
	SSD1306.currentX = x;
	SSD1306.currentY = y;
}

void ssd1306_drawPixel(uint16_t x, uint16_t y, uint8_t color) {
	if (
		x >= SSD1306_WIDTH ||
		y >= SSD1306_HEIGHT
	) {
		/* Error */
		return;
	}

	/* Set color */
	if (color == COLOR_ON) {
		SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] |= 1 << (y % 8);
	} else {
		SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
	}
}

char ssd1306_putc(char ch, FontDef_t* Font, uint8_t color) {
	uint32_t i, b, j;

	/* Check available space in LCD */
	if (
		SSD1306_WIDTH <= (SSD1306.currentX + Font->FontWidth) ||
		SSD1306_HEIGHT <= (SSD1306.currentY + Font->FontHeight)
	) {
		/* Error */
		return 0;
	}

	/* Go through font */
	for (i = 0; i < Font->FontHeight; i++) {
		b = Font->data[(ch - 32) * Font->FontHeight + i];
		for (j = 0; j < Font->FontWidth; j++) {
			if ((b << j) & 0x8000) {
				ssd1306_drawPixel(SSD1306.currentX + j, (SSD1306.currentY + i), color);
			} else {
				ssd1306_drawPixel(SSD1306.currentX + j, (SSD1306.currentY + i), !color);
			}
		}
	}

	/* Increase pointer */
	SSD1306.currentX += Font->FontWidth;

	/* Return character written */
	return ch;
}


char ssd1306_puts(char* str, FontDef_t* Font, uint8_t color) {
	/* Write characters */
	while (*str) {
		/* Write character by character */
		if (ssd1306_putc(*str, Font, color) != *str) {
			/* Return error */
			return *str;
		}

		/* Increase string pointer */
		str++;
	}

	/* Everything OK, zero should be returned */
	return *str;
}



bool ssd1306_updateScreen(void) {
	uint8_t m;
	bool ret;
	for (m = 0; m < 8; m++) {
		ssd1306_sendCommand(0xB0 + m);
		ssd1306_sendCommand(0x00);
		ssd1306_sendCommand(0x10);

    	ret = ssd1306_writeMultiReg(0x40, &SSD1306_Buffer[SSD1306_WIDTH * m], SSD1306_WIDTH/2);
    	if(!ret){
    		return false;
    	}

    	ssd1306_sendCommand(0xB0 + m);
		ssd1306_sendCommand(0x00);
		ssd1306_sendCommand(0x14);

		ret = ssd1306_writeMultiReg(0x40, &SSD1306_Buffer[SSD1306_WIDTH * m + SSD1306_WIDTH/2], SSD1306_WIDTH/2);
        if(!ret){
        	return false;
        }
	}
	return true;
}


