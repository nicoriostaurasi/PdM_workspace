/*
 * API_i2c.h
 *
 *  Created on: 13 abr 2026
 *      Author: nicol
 */

#ifndef API_INC_API_SCREEN_H_
#define API_INC_API_SCREEN_H_


#include "API_digital_angle_meter.h"
#include "API_angle.h"
#include <stdbool.h>

bool screen_start(void);
bool updateDigitalScreen(angles_t angle);
bool updateAnalogScreen(angles_t angle);


#endif /* API_INC_API_SCREEN_H_ */
