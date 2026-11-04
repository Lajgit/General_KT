#ifndef TEST_STM32F1XX_HAL_H
#define TEST_STM32F1XX_HAL_H

#include <stddef.h>
#include <stdint.h>

/* 仅替换硬件边界，业务扫描、组帧、CRC 与接收解帧使用工程源代码。 */
typedef struct { uint32_t IDR; } GPIO_TypeDef;
typedef enum { GPIO_PIN_RESET, GPIO_PIN_SET } GPIO_PinState;
typedef enum { HAL_OK, HAL_ERROR } HAL_StatusTypeDef;
typedef struct { uint32_t gState; } UART_HandleTypeDef;
typedef struct { uint32_t unused; } DMA_HandleTypeDef;
typedef struct { uint32_t unused; } TIM_HandleTypeDef;
extern GPIO_TypeDef test_gpio_a, test_gpio_b;
#define GPIOA (&test_gpio_a)
#define GPIOB (&test_gpio_b)
#define GPIO_PIN_0 0x0001u
#define GPIO_PIN_1 0x0002u
#define GPIO_PIN_2 0x0004u
#define GPIO_PIN_3 0x0008u
#define GPIO_PIN_4 0x0010u
#define GPIO_PIN_5 0x0020u
#define GPIO_PIN_6 0x0040u
#define GPIO_PIN_7 0x0080u
#define GPIO_PIN_8 0x0100u
#define GPIO_PIN_9 0x0200u
#define GPIO_PIN_10 0x0400u
#define GPIO_PIN_11 0x0800u
#define GPIO_PIN_12 0x1000u
#define GPIO_PIN_13 0x2000u
#define GPIO_PIN_14 0x4000u
#define GPIO_PIN_15 0x8000u
#define HAL_UART_STATE_READY 0x20u

uint32_t HAL_GetTick(void);
GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef *gpio, uint16_t pin);
HAL_StatusTypeDef HAL_UART_Transmit(UART_HandleTypeDef *uart, uint8_t *data, uint16_t size, uint32_t timeout);
HAL_StatusTypeDef HAL_UART_Receive_IT(UART_HandleTypeDef *uart, uint8_t *data, uint16_t size);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *uart);

#endif
