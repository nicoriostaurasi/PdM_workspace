/*
 * API_i2c.c
 *
 *  Created on: 12 abr 2026
 *      Author: nicol
 */

#include "API_i2c.h"
#include "stm32f4xx_hal.h"

static I2C_HandleTypeDef hi2c1;

bool i2c_memRead(uint16_t dev_addr, uint8_t reg, uint8_t* value, uint8_t size, uint32_t timeout){
	if(value == NULL){
		return false;
	}

	return (HAL_I2C_Mem_Read(&hi2c1,
			dev_addr,
            reg,
            I2C_MEMADD_SIZE_8BIT,
            value,
            size,
			timeout) == HAL_OK);
}

bool i2c_memWrite(uint16_t dev_addr, uint8_t reg, uint8_t* value, uint8_t size, uint32_t timeout){
	if(value == NULL){
		return false;
	}

	return (HAL_I2C_Mem_Write(&hi2c1,
			dev_addr,
            reg,
            I2C_MEMADD_SIZE_8BIT,
            value,
            size,
			timeout) == HAL_OK);
}

bool i2c_isDeviceReady(uint16_t DevAddress, uint32_t Trials, uint32_t Timeout){
	if(HAL_I2C_IsDeviceReady(&hi2c1, DevAddress, Trials, Timeout) == HAL_OK) {
		return true;
	}

	return false;
}

bool init_i2c_1(void){
	hi2c1.Instance = I2C1;
	hi2c1.Init.ClockSpeed = 100000;
	hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
	hi2c1.Init.OwnAddress1 = 0;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&hi2c1) != HAL_OK)
	{
		return false;
	}
}


