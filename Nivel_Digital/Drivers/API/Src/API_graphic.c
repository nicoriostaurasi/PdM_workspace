/*
 * API_graphic.c
 *
 *  Created on: 13 abr 2026
 *      Author: nicol
 */
#include <string.h>

#include "API_graphic.h"
#include "API_ssd1306.h"


#define GRAPHIC_WIDTH            128
#define GRAPHIC_HEIGHT           64

static uint8_t graphicBuffer[GRAPHIC_WIDTH * GRAPHIC_HEIGHT / 8];

typedef struct {
	uint16_t currentX;
	uint16_t currentY;
	uint8_t initialized;
} graphic_t;

static graphic_t graphic;
static display_t displayStructure;

void graphic_init(){
	graphic.currentX = 0;
	graphic.currentY = 0;

	graphic.initialized = 1;
	displayStructure.width = GRAPHIC_WIDTH;
    displayStructure.height = GRAPHIC_HEIGHT;
    displayStructure.buffer = graphicBuffer;

    graphic_fill(COLOR_OFF);
}

bool graphic_update(){
	return ssd1306_updateScreen(&displayStructure);
}

void graphic_fill(uint8_t value) {
	/* Set memory */
	memset(graphicBuffer, value , sizeof(graphicBuffer));
}

void graphic_gotoXY(uint16_t x, uint16_t y) {
	graphic.currentX = x;
	graphic.currentY = y;
}

void graphic_drawPixel(uint16_t x, uint16_t y, uint8_t color) {
	if (
		x >= GRAPHIC_WIDTH ||
		y >= GRAPHIC_HEIGHT
	) {
		/* Error */
		return;
	}

	/* Set color */
	if (color == COLOR_ON) {
		graphicBuffer[x + (y / 8) * GRAPHIC_WIDTH] |= 1 << (y % 8);
	} else {
		graphicBuffer[x + (y / 8) * GRAPHIC_WIDTH] &= ~(1 << (y % 8));
	}
}

char graphic_putc(char ch, FontDef_t* Font, uint8_t color) {
	uint32_t i, b, j;

	/* Check available space in LCD */
	if (
		GRAPHIC_WIDTH <= (graphic.currentX + Font->FontWidth) ||
		GRAPHIC_HEIGHT <= (graphic.currentY + Font->FontHeight)
	) {
		/* Error */
		return 0;
	}

	/* Go through font */
	for (i = 0; i < Font->FontHeight; i++) {
		b = Font->data[(ch - 32) * Font->FontHeight + i];
		for (j = 0; j < Font->FontWidth; j++) {
			if ((b << j) & 0x8000) {
				graphic_drawPixel(graphic.currentX + j, (graphic.currentY + i), color);
			} else {
				graphic_drawPixel(graphic.currentX + j, (graphic.currentY + i), !color);
			}
		}
	}

	/* Increase pointer */
	graphic.currentX += Font->FontWidth;

	/* Return character written */
	return ch;
}


char graphic_puts(char* str, FontDef_t* Font, uint8_t color) {
	/* Write characters */
	while (*str) {
		/* Write character by character */
		if (graphic_putc(*str, Font, color) != *str) {
			/* Return error */
			return *str;
		}

		/* Increase string pointer */
		str++;
	}

	/* Everything OK, zero should be returned */
	return *str;
}


void graphic_drawHLine(uint16_t x, uint16_t y, uint16_t w, uint8_t color) {
    for (uint16_t i = 0; i < w; i++) {
        graphic_drawPixel(x + i, y, color);
    }
}

void graphic_drawVLine(uint16_t x, uint16_t y, uint16_t h, uint8_t color) {
    for (uint16_t i = 0; i < h; i++) {
        graphic_drawPixel(x, y + i, color);
    }
}

void graphic_drawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t color) {
    if (w == 0 || h == 0) return;

    graphic_drawHLine(x, y, w, color);
    graphic_drawHLine(x, y + h - 1, w, color);
    graphic_drawVLine(x, y, h, color);
    graphic_drawVLine(x + w - 1, y, h, color);
}

void graphic_drawCircle(int16_t x0, int16_t y0, int16_t r, uint8_t color) {
    int16_t x = r;
    int16_t y = 0;
    int16_t err = 0;

    while (x >= y) {
        graphic_drawPixel(x0 + x, y0 + y, color);
        graphic_drawPixel(x0 + y, y0 + x, color);
        graphic_drawPixel(x0 - y, y0 + x, color);
        graphic_drawPixel(x0 - x, y0 + y, color);
        graphic_drawPixel(x0 - x, y0 - y, color);
        graphic_drawPixel(x0 - y, y0 - x, color);
        graphic_drawPixel(x0 + y, y0 - x, color);
        graphic_drawPixel(x0 + x, y0 - y, color);

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

void graphic_fillCircle(int16_t x0, int16_t y0, int16_t r, uint8_t color) {
    for (int16_t y = -r; y <= r; y++) {
        for (int16_t x = -r; x <= r; x++) {
            if ((x * x + y * y) <= (r * r)) {
                graphic_drawPixel(x0 + x, y0 + y, color);
            }
        }
    }
}


