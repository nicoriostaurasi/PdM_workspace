/*
 * API_screen.c
 *
 *  Created on: 13 abr 2026
 *      Author: nicol
 */
#include "API_screen.h"
#include <stdint.h>
#include "API_ssd1306.h"
#include "API_graphic.h"

bool screen_start(){
	graphic_init();
	return ssd1306_init();
}

static int16_t round_to_int(float x) {
    if (x >= 0.0f) return (int16_t)(x + 0.5f);
    return (int16_t)(x - 0.5f);
}

static float clampf(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

static void angle_to_string(float value, char *buf) {
    int16_t entero;
    uint16_t decimal;
    uint8_t idx = 0;

    if (value < 0.0f) {
        buf[idx++] = '-';
        value = -value;
    }

    entero = (int16_t)value;
    decimal = (uint16_t)((value - (float)entero) * 10.0f + 0.5f);

    if (decimal >= 10) {
        entero++;
        decimal = 0;
    }

    if (entero >= 100) {
        buf[idx++] = '0' + (entero / 100);
        buf[idx++] = '0' + ((entero / 10) % 10);
        buf[idx++] = '0' + (entero % 10);
    } else if (entero >= 10) {
        buf[idx++] = '0' + (entero / 10);
        buf[idx++] = '0' + (entero % 10);
    } else {
        buf[idx++] = '0' + entero;
    }

    buf[idx++] = '.';
    buf[idx++] = '0' + decimal;
    buf[idx++] = 0;
}


 void screen_drawDigitalInclinometer(angles_t angle) {
    char rollStr[8];
    char pitchStr[8];

    /* Parámetros del visor circular */
    const int16_t cx = 32;
    const int16_t cy = 32;
    const int16_t radius = 20;
    const int16_t bubbleRadius = 3;

    /* Se toma +/-45° como rango visual */
    const float maxVisualAngle = 45.0f;
    const int16_t maxOffset = radius - 5;

    float rollClamped;
    float pitchClamped;
    int16_t bubbleX;
    int16_t bubbleY;

    rollClamped = clampf(angle.roll, -maxVisualAngle, maxVisualAngle);
    pitchClamped = clampf(angle.pitch, -maxVisualAngle, maxVisualAngle);

    bubbleX = cx + round_to_int((rollClamped / maxVisualAngle) * maxOffset);
    bubbleY = cy - round_to_int((pitchClamped / maxVisualAngle) * maxOffset);

    angle_to_string(angle.roll, rollStr);
    angle_to_string(angle.pitch, pitchStr);

    ssd1306_fill(COLOR_OFF);

    /* Marco general */
    ssd1306_drawRect(0, 0, 128, 64, COLOR_ON);

    /* Título */
    ssd1306_gotoXY(4, 2);
    ssd1306_puts("INCLINOMETRO", &Font_7x10, COLOR_ON);

    /* Visor circular */
    ssd1306_drawCircle(cx, cy, radius, COLOR_ON);
    ssd1306_drawCircle(cx, cy, radius - 1, COLOR_ON);

    /* Referencias centrales */
    ssd1306_drawLine(cx - radius + 4, cy, cx + radius - 4, cy, COLOR_ON);
    ssd1306_drawLine(cx, cy - radius + 4, cx, cy + radius - 4, COLOR_ON);

    /* Marcas diagonales suaves */
    ssd1306_drawPixel(cx - 10, cy - 10, COLOR_ON);
    ssd1306_drawPixel(cx + 10, cy - 10, COLOR_ON);
    ssd1306_drawPixel(cx - 10, cy + 10, COLOR_ON);
    ssd1306_drawPixel(cx + 10, cy + 10, COLOR_ON);

    /* Burbuja */
    ssd1306_fillCircle(bubbleX, bubbleY, bubbleRadius, COLOR_ON);

    /* Punto central */
    ssd1306_drawPixel(cx, cy, COLOR_ON);

    /* Panel derecho */
    ssd1306_drawRect(70, 14, 54, 42, COLOR_ON);

    ssd1306_gotoXY(76, 18);
    ssd1306_puts("ROLL", &Font_7x10, COLOR_ON);
    ssd1306_gotoXY(76, 28);
    ssd1306_puts(rollStr, &Font_11x18, COLOR_ON);

    ssd1306_gotoXY(76, 48);
    ssd1306_puts("P", &Font_7x10, COLOR_ON);
    ssd1306_gotoXY(84, 48);
    ssd1306_puts(pitchStr, &Font_7x10, COLOR_ON);

    graphic_update();

    return;
}
