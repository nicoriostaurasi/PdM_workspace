/*
 * API_ssd1306.h
 *
 *  Created on: 12 abr 2026
 *      Author: nicol
 */

#ifndef API_INC_API_SSD1306_H_
#define API_INC_API_SSD1306_H_

#include "API_i2c.h"
#include <stdint.h>
#include <stdbool.h>
#include "fonts.h"

bool ssd1306_init(void);
bool ssd1306_sendCommand(uint8_t command);
bool ssd1306_writeReg(uint8_t reg, uint8_t value);
bool ssd1306_init(void);
void ssd1306_Fill(uint8_t value);
bool ssd1306_UpdateScreen(void);
void SSD1306_GotoXY(uint16_t x, uint16_t y);
char SSD1306_Puts(char* str, FontDef_t* Font, uint8_t color);
char SSD1306_Putc(char ch, FontDef_t* Font, uint8_t color);
void SSD1306_DrawPixel(uint16_t x, uint16_t y, uint8_t color);


#endif /* API_INC_API_SSD1306_H_ */
