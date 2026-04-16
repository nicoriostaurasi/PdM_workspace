/**
 * @file  BSP_gpios.c
 * @brief Configuracion e inicializacion de GPIOs (pulsador B1 y LED LD2).
 * @date 12 abr 2026
 * @author Ing. Nicolas Gabriel Rios Taurasi
 */

#include "BSP_gpios.h"
#include "stm32f4xx_hal.h"

#define B1_Pin GPIO_PIN_13              /**< Pin del botón de usuario B1 */
#define B1_GPIO_Port GPIOC              /**< Puerto del botón de usuario B1 */

#define LD2_Pin GPIO_PIN_5              /**< Pin del LED de usuario LD2 */
#define LD2_GPIO_Port GPIOA             /**< Puerto del LED de usuario LD2 */

/**
 * @brief Conmuta el estado del LED LD2.
 */
void gpios_toggleLed(void){
	HAL_GPIO_TogglePin(LD2_GPIO_Port,LD2_Pin);
}

/**
 * @brief Lee el estado eléctrico del botón de usuario B1.
 * @return true si el botón está presionado, false si está liberado.
 */
bool gpios_readButton(void){
	return (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) != GPIO_PIN_SET);
}

/**
 * @brief Inicializa los GPIOs del sistema.
 *
 * Habilita los relojes de los puertos A, B, C y H. Configura el pin B1
 * como entrada con interrupcion por flanco descendente (IT_FALLING) y
 * el pin LD2 como salida push-pull sin pull-up/pull-down.
 */
void gpios_init(void){
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

}
