#include "MainTask.h"
#include "CommTask.h"
#include "FlashTask.h"
#include "InterruptTask.h"
#include "KeyTask.h"
#include "MesgTask.h"
#include "port_event.h"
#include "DigitalTubeTask.h"
#include "iwdg.h"
#include "tim.h"

#define SYSLIGHT_BLINK_TIME 500
#define EncoderCheckTime 100

Event_Handle_t Event;
extern Tx_HandleTypeDef Tx;
void System_Reset(void)
{
    __disable_irq();
    HAL_NVIC_SystemReset();
}

static void SystemLight_Task(void)
{
    static uint32_t time = 0;
    if (HAL_GetTick() - time > SYSLIGHT_BLINK_TIME)
    {
        HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
        time = HAL_GetTick();
    }
}

static void Encoder_Task(void)
{
    static uint16_t last_CNT = 0;
    static uint32_t time = 0;
    if (HAL_GetTick() - time > EncoderCheckTime)
    {
        /* 以有符号差值判断方向，正确处理 16 位计数器回绕。 */
        uint16_t now_CNT = __HAL_TIM_GetCounter(&htim1);
        int16_t delta = (int16_t)(now_CNT - last_CNT);
        time = HAL_GetTick();
        if (delta > 0)
        {
            Comm_SendMesg_FillData(&Tx, Ctrl_to_Board, 0x03, 0x00, 0x00);
        }
        else if (delta < 0)
        {
            Comm_SendMesg_FillData(&Tx, Ctrl_to_Board, 0x03, 0x00, 0x01);
        }
        last_CNT = now_CNT;
    }
}

void Main_Init(void)
{
    CommInit();
    KeyAll_Init();
    DigitalTubeTask_Init();
    HAL_TIM_Encoder_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_Encoder_Start(&htim1, TIM_CHANNEL_2);
}

void Main_Task(void)
{
    CommTask();
    HAL_IWDG_Refresh(&hiwdg);
    Key_Task();
    HAL_IWDG_Refresh(&hiwdg);
    Mesg_Task();
    HAL_IWDG_Refresh(&hiwdg);
    DigitalTube_Task();
    HAL_IWDG_Refresh(&hiwdg);
    SystemLight_Task();
    Encoder_Task();
}