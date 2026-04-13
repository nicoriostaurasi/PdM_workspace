/*
 * API_i2c.h
 *
 *  Created on: 13 abr 2026
 *      Author: nicol
 */

#ifndef API_INC_API_SCREEN_H_
#define API_INC_API_SCREEN_H_


#include "API_digital_angle_meter.h"
#include <stdbool.h>

void screen_drawDigitalInclinometer(angles_t angle);
bool screen_start(void);


#endif /* API_INC_API_SCREEN_H_ */
