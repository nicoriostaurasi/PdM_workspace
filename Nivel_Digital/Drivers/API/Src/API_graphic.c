/*
 * API_graphic.c
 *
 *  Created on: 13 abr 2026
 *      Author: nicol
 */
#include "API_graphic.h"
#include "API_ssd1306.h"


#define SSD1306_WIDTH            128
#define SSD1306_HEIGHT           64

static uint8_t SSD1306_Buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];

typedef struct {
	uint16_t currentX;
	uint16_t currentY;
	uint8_t initialized;
} SSD1306_t;

static SSD1306_t SSD1306;
static display_t displayStructure;

void graphic_init(){
	SSD1306.currentX = 0;
	SSD1306.currentY = 0;

	SSD1306.initialized = 1;
	displayStructure.width = SSD1306_WIDTH;
    displayStructure.height = SSD1306_HEIGHT;
    displayStructure.buffer = SSD1306_Buffer;

	memset(SSD1306_Buffer,COLOR_OFF,sizeof(SSD1306_Buffer));
}

bool graphic_update(){
	return ssd1306_updateScreen(&displayStructure);
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


void ssd1306_drawHLine(uint16_t x, uint16_t y, uint16_t w, uint8_t color) {
    for (uint16_t i = 0; i < w; i++) {
        ssd1306_drawPixel(x + i, y, color);
    }
}

void ssd1306_drawVLine(uint16_t x, uint16_t y, uint16_t h, uint8_t color) {
    for (uint16_t i = 0; i < h; i++) {
        ssd1306_drawPixel(x, y + i, color);
    }
}

void ssd1306_drawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t color) {
    if (w == 0 || h == 0) return;

    ssd1306_drawHLine(x, y, w, color);
    ssd1306_drawHLine(x, y + h - 1, w, color);
    ssd1306_drawVLine(x, y, h, color);
    ssd1306_drawVLine(x + w - 1, y, h, color);
}

void ssd1306_drawCircle(int16_t x0, int16_t y0, int16_t r, uint8_t color) {
    int16_t x = r;
    int16_t y = 0;
    int16_t err = 0;

    while (x >= y) {
        ssd1306_drawPixel(x0 + x, y0 + y, color);
        ssd1306_drawPixel(x0 + y, y0 + x, color);
        ssd1306_drawPixel(x0 - y, y0 + x, color);
        ssd1306_drawPixel(x0 - x, y0 + y, color);
        ssd1306_drawPixel(x0 - x, y0 - y, color);
        ssd1306_drawPixel(x0 - y, y0 - x, color);
        ssd1306_drawPixel(x0 + y, y0 - x, color);
        ssd1306_drawPixel(x0 + x, y0 - y, color);

        y++;

        if (err <= 0) {
            err += 2 * y + 1;
        }

        if (err > 0) {
            x--;
            err -= 2 * x + 1;
        }
    }
}

void ssd1306_fillCircle(int16_t x0, int16_t y0, int16_t r, uint8_t color) {
    for (int16_t y = -r; y <= r; y++) {
        for (int16_t x = -r; x <= r; x++) {
            if ((x * x + y * y) <= (r * r)) {
                ssd1306_drawPixel(x0 + x, y0 + y, color);
            }
        }
    }
}


