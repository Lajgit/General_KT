/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "stm32f1xx_hal.h"

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
/* 通用控台原理图：输入均由外部电阻上拉，按下为低电平。 */
/* User_Led 接 PA1（芯片第 11 脚），低电平点亮。 */
#define LED_Pin GPIO_PIN_1
#define LED_GPIO_Port GPIOA
#define Encoder_A_Pin GPIO_PIN_8
#define Encoder_A_GPIO_Port GPIOA
#define Encoder_B_Pin GPIO_PIN_9
#define Encoder_B_GPIO_Port GPIOA
#define Joystick_Down_Pin GPIO_PIN_4
#define Joystick_Down_GPIO_Port GPIOA
#define Joystick_Up_Pin GPIO_PIN_5
#define Joystick_Up_GPIO_Port GPIOA
#define Joystick_Right_Pin GPIO_PIN_6
#define Joystick_Right_GPIO_Port GPIOA
#define Joystick_Left_Pin GPIO_PIN_7
#define Joystick_Left_GPIO_Port GPIOA
#define Button1_Pin GPIO_PIN_10
#define Button1_GPIO_Port GPIOA
#define Button2_Pin GPIO_PIN_11
#define Button2_GPIO_Port GPIOA
#define Button3_Pin GPIO_PIN_12
#define Button3_GPIO_Port GPIOA
#define Button4_Pin GPIO_PIN_5
#define Button4_GPIO_Port GPIOB
#define Button5_Pin GPIO_PIN_6
#define Button5_GPIO_Port GPIOB
#define Button6_Pin GPIO_PIN_7
#define Button6_GPIO_Port GPIOB
#define KeyBoard1_Pin GPIO_PIN_8
#define KeyBoard1_GPIO_Port GPIOB
#define KeyBoard2_Pin GPIO_PIN_9
#define KeyBoard2_GPIO_Port GPIOB
/* J4：PB12 为锁存，PB14 为输出使能；PB13/PB15 由 SPI2 驱动。 */
#define SPI2_LE_Pin GPIO_PIN_12
#define SPI2_LE_GPIO_Port GPIOB
#define SPI2_OE_Pin GPIO_PIN_14
#define SPI2_OE_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
