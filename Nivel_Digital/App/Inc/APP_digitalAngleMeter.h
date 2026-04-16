/**
 * @file    APP_digitalAngleMeter.h
 * @brief   FSM principal del inclinómetro digital. Orquesta sensor, display
 *          y comandos UART.
 *
 * @date    11 abr 2026
 * @author  Ing. Nicolás Gabriel Rios Taurasi
 */

#ifndef APP_INC_API_DIGITAL_ANGLE_METER_H_
#define APP_INC_API_DIGITAL_ANGLE_METER_H_

/**
 * @brief   Inicializa la FSM en su estado de arranque.
 *
 *          No inicializa los periféricos: eso ocurre dentro de la FSM al entrar
 *          por primera vez al estado FSM_INIT.
 */
void digitalAngleMeter_fsmInit(void);

/**
 * @brief   Ejecuta un tick de la FSM. Debe llamarse periódicamente desde el
 *          bucle principal.
 */
void digitalAngleMeter_fsmUpdate(void);

#endif /* APP_INC_API_DIGITAL_ANGLE_METER_H_ */
