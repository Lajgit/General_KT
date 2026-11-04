#ifndef __PORT_JOYSTICK_H__
#define __PORT_JOYSTICK_H__

#include "main.h"
#include <stdbool.h>

/* 方向码与主板协议一致，斜向表示两路触点同时有效。 */
typedef enum
{
    JOYSTICK_NONE = 0x00,
    JOYSTICK_UP = 0x01,
    JOYSTICK_DOWN = 0x02,
    JOYSTICK_LEFT = 0x03,
    JOYSTICK_RIGHT = 0x04,
    JOYSTICK_UP_LEFT = 0x05,
    JOYSTICK_DOWN_LEFT = 0x06,
    JOYSTICK_UP_RIGHT = 0x07,
    JOYSTICK_DOWN_RIGHT = 0x08,
} Joystick_Direction_t;

#define JOYSTICK_SHORT_PRESS 0x01
#define JOYSTICK_LONG_PRESS 0x02
#define JOYSTICK_RELEASE 0x03
#define JOYSTICK_DEBOUNCE_MS 15u
#define JOYSTICK_LONG_PRESS_MS 800u
#define JOYSTICK_REPEAT_MS (1000u / 3u)

typedef struct
{
    Joystick_Direction_t candidate; // 待消抖的完整方向
    Joystick_Direction_t direction; // 已确认的方向
    uint32_t changed_tick;          // 候选方向最后变化时间
    uint32_t press_tick;            // 方向按下时间
    uint32_t repeat_tick;           // 最近长按上报时间
    bool long_reported;             // 当前方向是否已上报长按
    void (*notify)(uint8_t direction, uint8_t event); // 方向事件回调
} Joystick_Handle_t;

void Joystick_Init(Joystick_Handle_t *joystick, void (*notify)(uint8_t, uint8_t));
void Joystick_Scan(Joystick_Handle_t *joystick);

#endif
