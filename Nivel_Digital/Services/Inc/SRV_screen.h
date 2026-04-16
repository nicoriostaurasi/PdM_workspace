/**
 * @file    SRV_screen.h
 * @brief   Vistas de alto nivel del inclinómetro sobre el display OLED.
 *
 * @date    13 abr 2026
 * @author  Ing. Nicolás Gabriel Rios Taurasi
 */

#ifndef SRV_INC_SRV_SCREEN_H_
#define SRV_INC_SRV_SCREEN_H_

#include <stdbool.h>

#include "SRV_angle.h"

/**
 * @brief   Inicializa el subsistema de pantalla (graphic + driver SSD1306).
 *
 * @return  true si la pantalla quedó inicializada, false en caso contrario.
 */
bool screen_start(void);

/**
 * @brief   Renderiza la vista digital (pitch/roll como texto en dos recuadros).
 *
 * @param   angle   Ángulos a mostrar.
 * @return  true si el frame se envió correctamente al display, false en caso contrario.
 */
bool screen_updateDigital(angles_t angle);

/**
 * @brief   Renderiza la vista analógica (visor circular con burbuja).
 *
 * @param   angle   Ángulos a mostrar.
 * @return  true si el frame se envió correctamente al display, false en caso contrario.
 */
bool screen_updateAnalog(angles_t angle);

/**
 * @brief   Chequeo de salud no invasivo de la pantalla OLED.
 *
 * @return  true si el display responde en el bus I2C, false en caso contrario.
 */
bool screen_isAlive(void);

#endif /* SRV_INC_SRV_SCREEN_H_ */
