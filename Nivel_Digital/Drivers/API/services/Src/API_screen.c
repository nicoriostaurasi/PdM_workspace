/** @file   API_screen.c
 *  @brief  Vistas de alto nivel del inclinómetro sobre el display OLED SSD1306.
 *
 *  Contiene las funciones de renderizado para las vistas digital (pitch/roll
 *  como texto) y analógica (visor circular con burbuja).
 *
 *  @date   13 abr 2026
 *  @author Nicolás Rios Taurasi
 */
#include <stdint.h>

#include "API_screen.h"
#include "API_ssd1306.h"
#include "API_graphic.h"

/** @brief  Inicializa el subsistema de pantalla (capa gráfica + driver SSD1306).
 *  @return true si la inicialización fue exitosa, false en caso contrario.
 */
bool screen_start(){
	graphic_init();
	return ssd1306_init();
}

/** @brief  Dibuja el recuadro doble decorativo que enmarca toda la pantalla. */
static void buildBox(){
	graphic_drawRect(2, 0, 126, 64, COLOR_ON);
	graphic_drawRect(2+1, 0+1, 126-2, 64-2, COLOR_ON);

}

/** @brief  Dibuja un recuadro con título y valor de ángulo para la vista digital.
 *
 *  @param  x       Coordenada X de la esquina superior izquierda del recuadro.
 *  @param  y       Coordenada Y de la esquina superior izquierda del recuadro.
 *  @param  tittle  Cadena con el nombre del eje (ej. "PITCH" o "ROLL").
 *  @param  angle   Cadena con el valor del ángulo ya formateado.
 */
static void buildAngleBox(uint16_t x, uint16_t y, char* tittle, char* angle){
	graphic_drawRect(x, y, 126/2-1, 64-2, COLOR_ON);
	graphic_drawHLine(x, y+1, 126/2-1, COLOR_ON);
	graphic_gotoXY(x+2, y+4);
	graphic_puts(tittle, &Font_7x10, COLOR_ON);
	graphic_gotoXY(x+2, y+15);
	graphic_puts(angle, &Font_11x18, COLOR_ON);

}

/** @brief  Dibuja el título centrado en la parte superior de la pantalla.
 *
 *  @param  tittle  Cadena con el texto del título. Si es NULL no dibuja nada.
 */
static void buildTittle(char* tittle){
	if(tittle==NULL){
		return;
	}
	graphic_gotoXY(4, 3);
    graphic_puts(tittle, &Font_7x10, COLOR_ON);
}

/** @brief  Convierte un valor flotante a cadena con un decimal de precisión.
 *
 *  Formato resultante: signo + dígitos enteros + '.' + 1 dígito decimal + '\0'.
 *
 *  @param  value  Valor flotante a convertir.
 *  @param  buf    Buffer de destino (debe tener al menos 8 bytes).
 */
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

/** @brief  Redondea un flotante al entero más cercano.
 *
 *  @param  x  Valor flotante a redondear.
 *  @return Entero de 16 bits con el valor redondeado.
 */
static int16_t round_to_int(float x) {
    if (x >= 0.0f) return (int16_t)(x + 0.5f);
    return (int16_t)(x - 0.5f);
}

/** @brief  Limita un valor flotante a un rango [min, max].
 *
 *  @param  value  Valor a limitar.
 *  @param  min    Límite inferior del rango.
 *  @param  max    Límite superior del rango.
 *  @return Valor acotado dentro del rango especificado.
 */
static float clampf(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

/** @brief  Dibuja la vista analógica del inclinómetro (visor circular con burbuja).
 *
 *  Representa los ángulos pitch y roll como el desplazamiento de una burbuja
 *  dentro de un visor circular, con un rango visual de +/-45 grados.
 *
 *  @param  angle  Estructura con los ángulos pitch y roll a representar.
 */
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
    graphic_fill(COLOR_OFF);

    buildBox();
    buildTittle("NIVEL ANALOGICO");


    rollClamped = clampf(angle.roll, -maxVisualAngle, maxVisualAngle);
    pitchClamped = clampf(angle.pitch, -maxVisualAngle, maxVisualAngle);

    bubbleX = cx + round_to_int((pitchClamped / maxVisualAngle) * maxOffset);
    bubbleY = cy - round_to_int((rollClamped / maxVisualAngle) * maxOffset);


    /* Visor circular */
    graphic_drawCircle(cx, cy, radius, COLOR_ON);
    graphic_drawCircle(cx, cy, radius - 1, COLOR_ON);

    /* Referencias centrales */
    graphic_drawHLine(cx-radius, cy, radius*2, COLOR_ON);
    graphic_drawVLine(cx, cy-radius , radius*2, COLOR_ON);

    /* Marcas diagonales suaves */
    graphic_drawPixel(cx - 10, cy - 10, COLOR_ON);
    graphic_drawPixel(cx + 10, cy - 10, COLOR_ON);
    graphic_drawPixel(cx - 10, cy + 10, COLOR_ON);
    graphic_drawPixel(cx + 10, cy + 10, COLOR_ON);
   /* Burbuja */

    graphic_fillCircle(bubbleX, bubbleY, bubbleRadius, COLOR_ON);

    /* Punto central */
    graphic_drawPixel(cx, cy, COLOR_ON);
}


/** @brief  Renderiza la vista digital mostrando pitch y roll como texto en dos recuadros.
 *
 *  @param  pitch  Ángulo de pitch en grados.
 *  @param  roll   Ángulo de roll en grados.
 */
static void screen_displayPitchRollDigital(float pitch, float roll)
{
     char spitch[12];
     char sroll[12];
     graphic_fill(COLOR_OFF);


     buildBox();
     buildTittle("NIVEL DIGITAL");
     floatToString(pitch, spitch);
     floatToString(roll, sroll);
     buildAngleBox(2+1,11+1, "PITCH",spitch);
     buildAngleBox(2+1+126/2-1,11+1, "ROLL", sroll);

}

/** @brief  Renderiza la vista analógica y envía el frame al display.
 *
 *  @param  angle  Estructura con los ángulos pitch y roll a representar.
 *  @return true si el frame se envió correctamente al display, false en caso contrario.
 */
bool screen_updateAnalog(angles_t angle){
	graphic_fill(COLOR_OFF);
	screen_drawAnalogInclinometer(angle);
	return graphic_update();
}

/** @brief  Renderiza la vista digital y envía el frame al display.
 *
 *  @param  angle  Estructura con los ángulos pitch y roll a representar.
 *  @return true si el frame se envió correctamente al display, false en caso contrario.
 */
bool screen_updateDigital(angles_t angle){
    graphic_fill(COLOR_OFF);
 	screen_displayPitchRollDigital(angle.pitch,angle.roll);
 	return graphic_update();
 }
