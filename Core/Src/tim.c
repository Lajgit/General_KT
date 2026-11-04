#include "tim.h"

/* PA8(CW)、PA9(CCW) 使用 TIM1 正交编码器接口。 */
TIM_HandleTypeDef htim1;

void MX_TIM1_Init(void)
{
    TIM_Encoder_InitTypeDef config = {0};
    TIM_MasterConfigTypeDef master = {0};

    htim1.Instance = TIM1;
    htim1.Init.Prescaler = 0;
    htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim1.Init.Period = 65535;
    htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.RepetitionCounter = 0;
    htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    /* 沿用参考控台的 TI1 计数方式和输入滤波。 */
    config.EncoderMode = TIM_ENCODERMODE_TI1;
    config.IC1Polarity = TIM_ICPOLARITY_RISING;
    config.IC1Selection = TIM_ICSELECTION_DIRECTTI;
    config.IC1Prescaler = TIM_ICPSC_DIV1;
    config.IC1Filter = 9;
    config.IC2Polarity = TIM_ICPOLARITY_RISING;
    config.IC2Selection = TIM_ICSELECTION_DIRECTTI;
    config.IC2Prescaler = TIM_ICPSC_DIV1;
    config.IC2Filter = 9;
    if (HAL_TIM_Encoder_Init(&htim1, &config) != HAL_OK)
        Error_Handler();

    master.MasterOutputTrigger = TIM_TRGO_RESET;
    master.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &master) != HAL_OK)
        Error_Handler();
}

void HAL_TIM_Encoder_MspInit(TIM_HandleTypeDef *timer)
{
    GPIO_InitTypeDef gpio = {0};
    if (timer->Instance == TIM1)
    {
        __HAL_RCC_TIM1_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        /* 旋钮有外部上拉，不再配置为旧板的 PWM 输出。 */
        gpio.Pin = Encoder_A_Pin|Encoder_B_Pin;
        gpio.Mode = GPIO_MODE_INPUT;
        gpio.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOA, &gpio);
    }
}

void HAL_TIM_Encoder_MspDeInit(TIM_HandleTypeDef *timer)
{
    if (timer->Instance == TIM1)
    {
        __HAL_RCC_TIM1_CLK_DISABLE();
        HAL_GPIO_DeInit(GPIOA, Encoder_A_Pin|Encoder_B_Pin);
    }
}
