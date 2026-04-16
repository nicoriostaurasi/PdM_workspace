/**
 * @file    BSP_gpios.h
 * @brief   Inicialización y utilidades básicas de GPIO de la placa.
 *
 * @date    12 abr 2026
 * @author  Ing. Nicolás Gabriel Rios Taurasi
 */

#ifndef BSP_INC_BSP_GPIOS_H_
#define BSP_INC_BSP_GPIOS_H_

#include <stdbool.h>

/**
 * @brief   Inicializa los GPIOs de la placa (LED de heartbeat y botón de usuario).
 */
void gpios_init(void);

/**
 * @brief   Alterna el estado del LED de heartbeat (LD2).
 */
void gpios_toggleLed(void);

/**
 * @brief   Lee el estado del botón de usuario B1.
 *
 * @return  true si el botón está presionado, false si está liberado.
 */
bool gpios_readButton(void);

#endif /* BSP_INC_BSP_GPIOS_H_ */
