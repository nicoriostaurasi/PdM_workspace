/*
 * API_gpios.h
 *
 *  Created on: 12 abr 2026
 *      Author: nicol
 */

#ifndef API_INC_API_GPIOS_H_
#define API_INC_API_GPIOS_H_

void board_gpios_init(void);

#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC

#define LD2_Pin GPIO_PIN_5
#define LD2_GPIO_Port GPIOA


#endif /* API_INC_API_GPIOS_H_ */
