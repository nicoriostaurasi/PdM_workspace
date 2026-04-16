/**
 * @file    API_graphic.h
 * @brief   Primitivas de dibujo sobre un framebuffer en RAM. El volcado al
 *          panel se hace con ssd1306_updateScreen() a través de graphic_update().
 *
 * @date    13 abr 2026
 * @author  nicol
 */

#ifndef API_INC_API_GRAPHIC_H_
#define API_INC_API_GRAPHIC_H_

#include <stdint.h>
#include <stdbool.h>

#include "fonts.h"

/**
 * @brief   Inicializa el framebuffer y el estado del cursor interno.
 */
void graphic_init(void);

/**
 * @brief   Envía el framebuffer actual al panel OLED.
 *
 * @return  true si la transferencia al display fue exitosa, false en caso contrario.
 */
bool graphic_update(void);

/**
 * @brief   Llena el framebuffer entero con el valor indicado.
 *
 * @param   value   Valor a escribir (COLOR_ON o COLOR_OFF).
 */
void graphic_fill(uint8_t value);

/**
 * @brief   Posiciona el cursor de texto en las coordenadas (x, y).
 *
 * @param   x   Coordenada X en píxeles.
 * @param   y   Coordenada Y en píxeles.
 */
void graphic_gotoXY(uint16_t x, uint16_t y);

/**
 * @brief   Dibuja una cadena en la posición actual del cursor, con la fuente
 *          y color indicados.
 *
 * @param   str     Cadena terminada en '\0' a dibujar.
 * @param   Font    Puntero al descriptor de fuente a usar.
 * @param   color   COLOR_ON u COLOR_OFF.
 * @return  0 si se dibujaron todos los caracteres; en caso de error, el
 *          caracter que no pudo ser dibujado.
 */
char graphic_puts(char* str, FontDef_t* Font, uint8_t color);

/**
 * @brief   Dibuja un único caracter en la posición actual del cursor.
 *
 * @param   ch      Caracter ASCII a dibujar.
 * @param   Font    Puntero al descriptor de fuente.
 * @param   color   COLOR_ON u COLOR_OFF.
 * @return  El caracter dibujado, o 0 si no entraba en el área visible.
 */
char graphic_putc(char ch, FontDef_t* Font, uint8_t color);

/**
 * @brief   Pinta un pixel en el framebuffer.
 *
 * @param   x       Coordenada X.
 * @param   y       Coordenada Y.
 * @param   color   COLOR_ON u COLOR_OFF.
 */
void graphic_drawPixel(uint16_t x, uint16_t y, uint8_t color);

/**
 * @brief   Dibuja el contorno de un rectángulo.
 *
 * @param   x       X de la esquina superior izquierda.
 * @param   y       Y de la esquina superior izquierda.
 * @param   w       Ancho en píxeles.
 * @param   h       Alto en píxeles.
 * @param   color   COLOR_ON u COLOR_OFF.
 */
void graphic_drawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t color);

/**
 * @brief   Dibuja el contorno de una circunferencia (algoritmo de Bresenham).
 *
 * @param   x0      Coordenada X del centro.
 * @param   y0      Coordenada Y del centro.
 * @param   r       Radio en píxeles.
 * @param   color   COLOR_ON u COLOR_OFF.
 */
void graphic_drawCircle(int16_t x0, int16_t y0, int16_t r, uint8_t color);

/**
 * @brief   Dibuja un círculo relleno (pinta todos los píxeles dentro del radio).
 *
 * @param   x0      Coordenada X del centro.
 * @param   y0      Coordenada Y del centro.
 * @param   r       Radio en píxeles.
 * @param   color   COLOR_ON u COLOR_OFF.
 */
void graphic_fillCircle(int16_t x0, int16_t y0, int16_t r, uint8_t color);

/**
 * @brief   Dibuja una línea horizontal.
 *
 * @param   x       X inicial.
 * @param   y       Y (fija para toda la línea).
 * @param   w       Largo en píxeles.
 * @param   color   COLOR_ON u COLOR_OFF.
 */
void graphic_drawHLine(uint16_t x, uint16_t y, uint16_t w, uint8_t color);

/**
 * @brief   Dibuja una línea vertical.
 *
 * @param   x       X (fija para toda la línea).
 * @param   y       Y inicial.
 * @param   w       Largo en píxeles.
 * @param   color   COLOR_ON u COLOR_OFF.
 */
void graphic_drawVLine(uint16_t x, uint16_t y, uint16_t w, uint8_t color);


#endif /* API_INC_API_GRAPHIC_H_ */
