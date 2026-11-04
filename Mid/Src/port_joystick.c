#include "port_joystick.h"

static Joystick_Direction_t Joystick_ReadDirection(void)
{
    /* 四路都在 GPIOA，一次采样避免组合方向读取过程中出现不一致。 */
    uint16_t pins = (uint16_t)~GPIOA->IDR &
                    (Joystick_Up_Pin | Joystick_Down_Pin | Joystick_Left_Pin | Joystick_Right_Pin);
    switch (pins)
    {
    case Joystick_Up_Pin: return JOYSTICK_UP;
    case Joystick_Down_Pin: return JOYSTICK_DOWN;
    case Joystick_Left_Pin: return JOYSTICK_LEFT;
    case Joystick_Right_Pin: return JOYSTICK_RIGHT;
    case Joystick_Up_Pin | Joystick_Left_Pin: return JOYSTICK_UP_LEFT;
    case Joystick_Down_Pin | Joystick_Left_Pin: return JOYSTICK_DOWN_LEFT;
    case Joystick_Up_Pin | Joystick_Right_Pin: return JOYSTICK_UP_RIGHT;
    case Joystick_Down_Pin | Joystick_Right_Pin: return JOYSTICK_DOWN_RIGHT;
    /* 相反方向或三路以上同时有效属于非法组合，按回中处理。 */
    default: return JOYSTICK_NONE;
    }
}

static void Joystick_Notify(Joystick_Handle_t *joystick, uint8_t event)
{
    if (joystick->notify != NULL)
        joystick->notify((uint8_t)joystick->direction, event);
}

void Joystick_Init(Joystick_Handle_t *joystick, void (*notify)(uint8_t, uint8_t))
{
    if (joystick == NULL)
        return;
    joystick->candidate = JOYSTICK_NONE;
    joystick->direction = JOYSTICK_NONE;
    joystick->changed_tick = HAL_GetTick();
    joystick->press_tick = joystick->changed_tick;
    joystick->repeat_tick = joystick->changed_tick;
    joystick->long_reported = false;
    joystick->notify = notify;
}

void Joystick_Scan(Joystick_Handle_t *joystick)
{
    if (joystick == NULL)
        return;
    uint32_t now = HAL_GetTick();
    Joystick_Direction_t sampled = Joystick_ReadDirection();
    if (sampled != joystick->candidate)
    {
        joystick->candidate = sampled;
        joystick->changed_tick = now;
    }

    if (joystick->candidate != joystick->direction)
    {
        /* 对完整方向消抖，将相近时刻闭合的两个触点合成为斜向。 */
        if (now - joystick->changed_tick < JOYSTICK_DEBOUNCE_MS)
            return;
        if (joystick->direction != JOYSTICK_NONE)
        {
            if (!joystick->long_reported)
            {
                /* 沿用原控台的短按判定：松开或转向时上报短按。 */
                uint32_t held = joystick->changed_tick - joystick->press_tick;
                Joystick_Notify(joystick, held >= JOYSTICK_LONG_PRESS_MS ?
                                JOYSTICK_LONG_PRESS : JOYSTICK_SHORT_PRESS);
            }
            /* 松开必须携带旧方向，不能用回中码 0x00 替代。 */
            Joystick_Notify(joystick, JOYSTICK_RELEASE);
        }
        joystick->direction = joystick->candidate;
        joystick->press_tick = joystick->changed_tick;
        joystick->repeat_tick = joystick->changed_tick;
        joystick->long_reported = false;
    }

    if (joystick->direction == JOYSTICK_NONE)
        return;
    /* 无符号时间差同时支持系统毫秒计数器回绕。 */
    if (!joystick->long_reported && now - joystick->press_tick >= JOYSTICK_LONG_PRESS_MS)
    {
        joystick->long_reported = true;
        joystick->repeat_tick = now;
        Joystick_Notify(joystick, JOYSTICK_LONG_PRESS);
    }
    else if (joystick->long_reported && now - joystick->repeat_tick >= JOYSTICK_REPEAT_MS)
    {
        joystick->repeat_tick = now;
        Joystick_Notify(joystick, JOYSTICK_LONG_PRESS);
    }
}
