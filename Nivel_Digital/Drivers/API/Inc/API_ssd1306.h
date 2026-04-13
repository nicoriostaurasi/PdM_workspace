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

#define COLOR_ON 0x01
#define COLOR_OFF 0x00


typedef struct {
    uint16_t width;
    uint16_t height;
    uint8_t *buffer;
} display_t;

bool ssd1306_init(void);
void ssd1306_fill(uint8_t value);
bool ssd1306_updateScreen(display_t *disp);

#endif /* API_INC_API_SSD1306_H_ */
