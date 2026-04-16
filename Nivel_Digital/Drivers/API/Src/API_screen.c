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

static void buildBox(){
	ssd1306_drawRect(2, 0, 126, 64, COLOR_ON);
	ssd1306_drawRect(2+1, 0+1, 126-2, 64-2, COLOR_ON);

}

static void buildAngleBox(uint16_t x, uint16_t y, char* tittle, char* angle){
	ssd1306_drawRect(x, y, 126/2-1, 64-2, COLOR_ON);
	ssd1306_drawHLine(x, y+1, 126/2-1, COLOR_ON);
	ssd1306_gotoXY(x+2, y+4);
	ssd1306_puts(tittle, &Font_7x10, COLOR_ON);
	ssd1306_gotoXY(x+2, y+15);
	ssd1306_puts(angle, &Font_11x18, COLOR_ON);

}

static void buildTittle(char* tittle){
	if(tittle==NULL){
		return;
	}
	ssd1306_gotoXY(4, 3);
    ssd1306_puts(tittle, &Font_7x10, COLOR_ON);
}

static void floatToString(float value, char *buf){
    int16_t entero;
    uint16_t decimal;
    uint8_t idx = 0;

    if (value < 0.0f) {
        buf[idx++] = '-';
        value = -value;
    }else{
    	buf[idx++] = ' ';
        value = value;
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
    } else{
        buf[idx++] = '0' + (entero / 10);
        buf[idx++] = '0' + (entero % 10);
    }

    buf[idx++] = '.';
    buf[idx++] = '0' + decimal;
    buf[idx++] = 0;
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

static void screen_drawAnalogInclinometer(angles_t angle) {
    /* Parámetros del visor circular */
    const int16_t cx = 64;
    const int16_t cy = 37;
    const int16_t radius = 21;
    const int16_t bubbleRadius = 3;

    /* Se toma +/-45° como rango visual */
    const float maxVisualAngle = 45.0f;
    const int16_t maxOffset = radius - 5;

    float rollClamped;
    float pitchClamped;
    int16_t bubbleX;
    int16_t bubbleY;
    ssd1306_fill(COLOR_OFF);

    buildBox();
    buildTittle("NIVEL ANALOGICO");


    rollClamped = clampf(angle.roll, -maxVisualAngle, maxVisualAngle);
    pitchClamped = clampf(angle.pitch, -maxVisualAngle, maxVisualAngle);

    bubbleX = cx + round_to_int((pitchClamped / maxVisualAngle) * maxOffset);
    bubbleY = cy - round_to_int((rollClamped / maxVisualAngle) * maxOffset);


    /* Visor circular */
    ssd1306_drawCircle(cx, cy, radius, COLOR_ON);
    ssd1306_drawCircle(cx, cy, radius - 1, COLOR_ON);

    /* Referencias centrales */
    ssd1306_drawHLine(cx-radius, cy, radius*2, COLOR_ON);
    ssd1306_drawVLine(cx, cy-radius , radius*2, COLOR_ON);

    /* Marcas diagonales suaves */
    ssd1306_drawPixel(cx - 10, cy - 10, COLOR_ON);
    ssd1306_drawPixel(cx + 10, cy - 10, COLOR_ON);
    ssd1306_drawPixel(cx - 10, cy + 10, COLOR_ON);
    ssd1306_drawPixel(cx + 10, cy + 10, COLOR_ON);
   /* Burbuja */

    ssd1306_fillCircle(bubbleX, bubbleY, bubbleRadius, COLOR_ON);

    /* Punto central */
    ssd1306_drawPixel(cx, cy, COLOR_ON);
}


static void displayPitchRollDigital(float pitch, float roll)
{
     char spitch[12];
     char sroll[12];
     ssd1306_fill(COLOR_OFF);


     buildBox();
     buildTittle("NIVEL DIGITAL");
     floatToString(pitch, spitch);
     floatToString(roll, sroll);
     buildAngleBox(2+1,11+1, "PITCH",spitch);
     buildAngleBox(2+1+126/2-1,11+1, "ROLL", sroll);

}

bool updateAnalogScreen(angles_t angle){
	ssd1306_fill(COLOR_OFF);
	screen_drawAnalogInclinometer(angle);
	return graphic_update();
}

bool updateDigitalScreen(angles_t angle){
    ssd1306_fill(COLOR_OFF);
 	displayPitchRollDigital(angle.pitch,angle.roll);
 	return graphic_update();
 }

