/**
 * @file    API_graphic.c
 * @brief   Primitivas de dibujo sobre framebuffer en RAM para display SSD1306.
 *
 * @date    13 abr 2026
 * @author  nicol
 */
#include <string.h>

#include "API_graphic.h"
#include "API_ssd1306.h"


#define GRAPHIC_WIDTH            128     /**< Ancho del display en píxeles */
#define GRAPHIC_HEIGHT           64      /**< Alto del display en píxeles */

/** @brief  Framebuffer: 1 bit por pixel, organizado en páginas de 8 filas. */
static uint8_t graphicBuffer[GRAPHIC_WIDTH * GRAPHIC_HEIGHT / 8];

/**
 * @brief   Estado interno del módulo gráfico (posición de cursor, flag init).
 */
typedef struct {
	uint16_t currentX;      /**< Posición X actual del cursor de texto */
	uint16_t currentY;      /**< Posición Y actual del cursor de texto */
	uint8_t initialized;    /**< 1 si el módulo fue inicializado */
} graphic_t;

static graphic_t graphic;              /**< Instancia del estado gráfico */
static display_t displayStructure;     /**< Descriptor de display para el driver SSD1306 */

/**
 * @brief   Inicializa el framebuffer, el cursor y el descriptor de display.
 */
void graphic_init(){
	graphic.currentX = 0;
	graphic.currentY = 0;

	graphic.initialized = 1;
	displayStructure.width = GRAPHIC_WIDTH;
    displayStructure.height = GRAPHIC_HEIGHT;
    displayStructure.buffer = graphicBuffer;

    graphic_fill(COLOR_OFF);
}

/**
 * @brief   Envía el framebuffer actual al panel OLED vía ssd1306_updateScreen().
 *
 * @return  true si la transferencia fue exitosa, false en caso contrario.
 */
bool graphic_update(){
	return ssd1306_updateScreen(&displayStructure);
}

/**
 * @brief   Llena el framebuffer entero con el valor indicado.
 *
 * @param   value   Valor a escribir en cada byte (COLOR_ON o COLOR_OFF).
 */
void graphic_fill(uint8_t value) {
	memset(graphicBuffer, value , sizeof(graphicBuffer));
}

/**
 * @brief   Posiciona el cursor de texto en las coordenadas (x, y).
 *
 * @param   x   Coordenada X en píxeles.
 * @param   y   Coordenada Y en píxeles.
 */
void graphic_gotoXY(uint16_t x, uint16_t y) {
	graphic.currentX = x;
	graphic.currentY = y;
}

/**
 * @brief   Pinta o apaga un pixel en el framebuffer.
 *
 * @param   x       Coordenada X (0..GRAPHIC_WIDTH-1).
 * @param   y       Coordenada Y (0..GRAPHIC_HEIGHT-1).
 * @param   color   COLOR_ON para encender, COLOR_OFF para apagar.
 */
void graphic_drawPixel(uint16_t x, uint16_t y, uint8_t color) {
	if (
		x >= GRAPHIC_WIDTH ||
		y >= GRAPHIC_HEIGHT
	) {
		return;
	}

	if (color == COLOR_ON) {
		graphicBuffer[x + (y / 8) * GRAPHIC_WIDTH] |= 1 << (y % 8);
	} else {
		graphicBuffer[x + (y / 8) * GRAPHIC_WIDTH] &= ~(1 << (y % 8));
	}
}

/**
 * @brief   Dibuja un único caracter ASCII en la posición actual del cursor y
 *          avanza el cursor un ancho de fuente.
 *
 * @param   ch      Caracter ASCII a dibujar (offset desde 32).
 * @param   Font    Puntero al descriptor de fuente.
 * @param   color   COLOR_ON u COLOR_OFF.
 * @return  El caracter dibujado, o 0 si no entraba en el área visible.
 */
char graphic_putc(char ch, FontDef_t* Font, uint8_t color) {
	uint32_t i, b, j;

	if (
		GRAPHIC_WIDTH <= (graphic.currentX + Font->FontWidth) ||
		GRAPHIC_HEIGHT <= (graphic.currentY + Font->FontHeight)
	) {
		return 0;
	}

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

	graphic.currentX += Font->FontWidth;

	return ch;
}

/**
 * @brief   Dibuja una cadena terminada en '\0' en la posición actual del cursor.
 *
 * @param   str     Cadena a dibujar.
 * @param   Font    Puntero al descriptor de fuente.
 * @param   color   COLOR_ON u COLOR_OFF.
 * @return  0 si se dibujaron todos los caracteres; en caso de error, el
 *          caracter que no pudo ser dibujado.
 */
char graphic_puts(char* str, FontDef_t* Font, uint8_t color) {
	while (*str) {
		if (graphic_putc(*str, Font, color) != *str) {
			return *str;
		}
		str++;
	}

	return *str;
}

/**
 * @brief   Dibuja una línea horizontal.
 *
 * @param   x       X inicial.
 * @param   y       Y de la línea.
 * @param   w       Largo en píxeles.
 * @param   color   COLOR_ON u COLOR_OFF.
 */
void graphic_drawHLine(uint16_t x, uint16_t y, uint16_t w, uint8_t color) {
    for (uint16_t i = 0; i < w; i++) {
        graphic_drawPixel(x + i, y, color);
    }
}

/**
 * @brief   Dibuja una línea vertical.
 *
 * @param   x       X de la línea.
 * @param   y       Y inicial.
 * @param   h       Largo en píxeles.
 * @param   color   COLOR_ON u COLOR_OFF.
 */
void graphic_drawVLine(uint16_t x, uint16_t y, uint16_t h, uint8_t color) {
    for (uint16_t i = 0; i < h; i++) {
        graphic_drawPixel(x, y + i, color);
    }
}

/**
 * @brief   Dibuja el contorno de un rectángulo.
 *
 * @param   x       X de la esquina superior izquierda.
 * @param   y       Y de la esquina superior izquierda.
 * @param   w       Ancho en píxeles.
 * @param   h       Alto en píxeles.
 * @param   color   COLOR_ON u COLOR_OFF.
 */
void graphic_drawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t color) {
    if (w == 0 || h == 0) return;

    graphic_drawHLine(x, y, w, color);
    graphic_drawHLine(x, y + h - 1, w, color);
    graphic_drawVLine(x, y, h, color);
    graphic_drawVLine(x + w - 1, y, h, color);
}

/**
 * @brief   Dibuja el contorno de una circunferencia (algoritmo de Bresenham).
 *
 * @param   x0      Coordenada X del centro.
 * @param   y0      Coordenada Y del centro.
 * @param   r       Radio en píxeles.
 * @param   color   COLOR_ON u COLOR_OFF.
 */
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

/**
 * @brief   Dibuja un círculo relleno (todos los píxeles dentro del radio).
 *
 * @param   x0      Coordenada X del centro.
 * @param   y0      Coordenada Y del centro.
 * @param   r       Radio en píxeles.
 * @param   color   COLOR_ON u COLOR_OFF.
 */
void graphic_fillCircle(int16_t x0, int16_t y0, int16_t r, uint8_t color) {
    for (int16_t y = -r; y <= r; y++) {
        for (int16_t x = -r; x <= r; x++) {
            if ((x * x + y * y) <= (r * r)) {
                graphic_drawPixel(x0 + x, y0 + y, color);
            }
        }
    }
}
