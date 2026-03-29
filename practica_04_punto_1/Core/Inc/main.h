/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include <stdbool.h>
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define LD2_Pin GPIO_PIN_5
#define LD2_GPIO_Port GPIOA
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB

#define MS_TO_TICK 1
#define _100_MS_TO_TICK (100*MS_TO_TICK)
#define _500_MS_TO_TICK (500*MS_TO_TICK)
#define _1000_MS_TO_TICK (1000*MS_TO_TICK)
#define DEBOUNCER_SAMPLE_RATE (5*MS_TO_TICK)
#define DEBOUNCER_TIME (40*MS_TO_TICK)
#define DEBOUNCE_COUNTER_MAX (DEBOUNCER_TIME/DEBOUNCER_SAMPLE_RATE)

/** @brief Estados de la máquina de estados del debounce */
typedef enum{
BUTTON_UP,
BUTTON_FALLING,
BUTTON_DOWN,
BUTTON_RAISING,
} debounceState_t;

/** @brief Inicializa la máquina de estados del debounce
  * @retval none
  */
void debounceFSM_init(void);

/** @brief Actualiza la máquina de estados del debounce
  * @retval none
  */
void debounceFSM_update(void);

/** @brief Función que se ejecuta cuando se presiona el botón
  * @retval none
  */
void buttonPressed(void);

/** @brief Función que se ejecuta cuando se libera el botón
  * @retval none
  */
void buttonReleased(void);

/** @brief Función que enciende el led de la placa 
  * @retval none
  */
void boardLedOn(void);

/** @brief Función que apaga el led de la placa 
  * @retval none
  */
void boardLedOff(void);

/** @brief Función que verifica el estado del botón sin debounce
  * @retval true si el botón está presionado, false si no lo está
  */
bool checkButtonStatusPressedRaw(void);


/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
