/*
 * API_graphic.c
 *
 *  Created on: 13 abr 2026
 *      Author: nicol
 */

#ifndef API_INC_API_GRAPHIC_H_
#define API_INC_API_GRAPHIC_H_

#include <stdint.h>
#include "fonts.h"
#include <stdbool.h>

void ssd1306_gotoXY(uint16_t x, uint16_t y);
char ssd1306_puts(char* str, FontDef_t* Font, uint8_t color);
char ssd1306_putc(char ch, FontDef_t* Font, uint8_t color);
void ssd1306_drawPixel(uint16_t x, uint16_t y, uint8_t color);
void ssd1306_drawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t color);
void ssd1306_drawCircle(int16_t x0, int16_t y0, int16_t r, uint8_t color);
void ssd1306_drawLine(int x0, int y0, int x1, int y1, uint8_t color);
void ssd1306_fillCircle(int16_t x0, int16_t y0, int16_t r, uint8_t color);
void graphic_init(void);
bool graphic_update(void);



#endif /* API_INC_API_GRAPHIC_C_ */
