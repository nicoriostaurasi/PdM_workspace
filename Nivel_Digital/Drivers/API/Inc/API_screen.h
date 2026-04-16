/*
 * API_i2c.h
 *
 *  Created on: 13 abr 2026
 *      Author: nicol
 */

#ifndef API_INC_API_SCREEN_H_
#define API_INC_API_SCREEN_H_

#include <stdbool.h>

#include "API_digitalAngleMeter.h"
#include "API_angle.h"

bool screen_start(void);
bool screen_updateDigital(angles_t angle);
bool screen_updateAnalog(angles_t angle);


#endif /* API_INC_API_SCREEN_H_ */
