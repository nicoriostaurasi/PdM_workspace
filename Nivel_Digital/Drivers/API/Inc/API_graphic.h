/*
 * API_graphic.c
 *
 *  Created on: 13 abr 2026
 *      Author: nicol
 */

#ifndef API_INC_API_GRAPHIC_H_
#define API_INC_API_GRAPHIC_H_

#include <stdint.h>
#include <stdbool.h>

#include "fonts.h"

void graphic_init(void);
bool graphic_update(void);
void graphic_fill(uint8_t value);
void graphic_gotoXY(uint16_t x, uint16_t y);
char graphic_puts(char* str, FontDef_t* Font, uint8_t color);
char graphic_putc(char ch, FontDef_t* Font, uint8_t color);
void graphic_drawPixel(uint16_t x, uint16_t y, uint8_t color);
void graphic_drawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t color);
void graphic_drawCircle(int16_t x0, int16_t y0, int16_t r, uint8_t color);
void graphic_fillCircle(int16_t x0, int16_t y0, int16_t r, uint8_t color);
void graphic_drawHLine(uint16_t x, uint16_t y, uint16_t w, uint8_t color);
void graphic_drawVLine(uint16_t x, uint16_t y, uint16_t w, uint8_t color);



#endif /* API_INC_API_GRAPHIC_C_ */
