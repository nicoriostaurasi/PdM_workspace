/**
 * @file    PER_ssd1306.h
 * @brief   Driver del panel OLED SSD1306 (I2C).
 *
 * @date    12 abr 2026
 * @author Ing. Nicolás Gabriel Rios Taurasi
 */

#ifndef PER_INC_PER_SSD1306_H_
#define PER_INC_PER_SSD1306_H_

#include <stdint.h>
#include <stdbool.h>

#define COLOR_ON    0x01        /**< Pixel encendido */
#define COLOR_OFF   0x00        /**< Pixel apagado */


/**
 * @brief   Descripción del framebuffer que el driver debe volcar al panel.
 */
typedef struct {
    uint16_t width;         /**< Ancho del display, en píxeles */
    uint16_t height;        /**< Alto del display, en píxeles */
    uint8_t *buffer;        /**< Puntero al framebuffer (bitpacked por páginas) */
} display_t;

/**
 * @brief   Inicializa el panel SSD1306 (secuencia de comandos estándar).
 *
 * @return  true si el panel respondió y se configuró correctamente, false en
 *          caso contrario.
 */
bool ssd1306_init(void);

/**
 * @brief   Vuelca el framebuffer al panel.
 *
 * @param   disp    Puntero al descriptor de display con el buffer a enviar.
 * @return  true si la transferencia fue exitosa, false si @p disp es NULL o
 *          hubo error en el bus I2C.
 */
bool ssd1306_updateScreen(display_t *disp);

/**
 * @brief   Chequeo de salud no invasivo: ping I2C al display sin reconfigurarlo.
 *
 * @return  true si el display responde en el bus, false en caso contrario.
 */
bool ssd1306_isAlive(void);

#endif /* PER_INC_PER_SSD1306_H_ */
