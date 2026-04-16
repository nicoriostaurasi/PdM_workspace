/**
 * @file    API_gpios.h
 * @brief   Inicialización y utilidades básicas de GPIO de la placa.
 *
 * @date    12 abr 2026
 * @author  nicol
 */

#ifndef API_INC_API_GPIOS_H_
#define API_INC_API_GPIOS_H_

#define B1_Pin GPIO_PIN_13              /**< Pin del botón de usuario B1 */
#define B1_GPIO_Port GPIOC              /**< Puerto del botón de usuario B1 */

#define LD2_Pin GPIO_PIN_5              /**< Pin del LED de usuario LD2 */
#define LD2_GPIO_Port GPIOA             /**< Puerto del LED de usuario LD2 */

/**
 * @brief   Inicializa los GPIOs de la placa (LED de heartbeat y botón de usuario).
 */
void gpios_init(void);

/**
 * @brief   Alterna el estado del LED de heartbeat (LD2).
 */
void gpios_toggleLed(void);


#endif /* API_INC_API_GPIOS_H_ */
