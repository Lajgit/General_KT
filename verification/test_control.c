#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "CommTask.h"
#include "KeyTask.h"
#include "port_joystick.h"
#include "port_digitaltube.h"
#include "usart.h"

GPIO_TypeDef test_gpio_a, test_gpio_b;
UART_HandleTypeDef huart2;
DigitalTube_t DigitalTube;
extern Tx_HandleTypeDef Tx;
extern Rx_HandleTypeDef Rx;
static uint32_t tick, base;
static uint8_t frames[300][14];
static unsigned frame_count, display_count, refresh_count;
static uint32_t display_value;
static uint8_t *receive_byte;

uint32_t HAL_GetTick(void) { return tick; }
GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef *gpio, uint16_t pin)
{
    return (gpio->IDR & pin) ? GPIO_PIN_SET : GPIO_PIN_RESET;
}
HAL_StatusTypeDef HAL_UART_Transmit(UART_HandleTypeDef *uart, uint8_t *data, uint16_t size, uint32_t timeout)
{
    (void)timeout;
    assert(uart == &huart2 && size == 14 && frame_count < 300);
    memcpy(frames[frame_count++], data, size);
    return HAL_OK;
}
HAL_StatusTypeDef HAL_UART_Receive_IT(UART_HandleTypeDef *uart, uint8_t *data, uint16_t size)
{
    assert(uart == &huart2 && size == 1);
    receive_byte = data;
    return HAL_OK;
}
static void set_display(void *self, uint8_t position, uint32_t value, uint8_t size)
{
    assert(self == &DigitalTube && position == 0 && size == 4);
    display_count++;
    display_value = value;
}
static void refresh_display(void *self)
{
    assert(self == &DigitalTube);
    refresh_count++;
}

/* 独立 CRC 实现，并用标准校验向量和完整报文校验固件发送结果。 */
static uint16_t crc16(const uint8_t *data, size_t count)
{
    uint16_t crc = 0xffff;
    while (count--)
    {
        crc ^= *data++;
        for (unsigned bit = 0; bit < 8; bit++)
            crc = (crc & 1u) ? (crc >> 1) ^ 0xa001u : crc >> 1;
    }
    return crc;
}
static void reset_test(uint32_t start)
{
    base = tick = start;
    test_gpio_a.IDR = test_gpio_b.IDR = 0xffff;
    frame_count = display_count = refresh_count = 0;
    CommInit();
    KeyAll_Init();
    DigitalTube.Set_Num = set_display;
    DigitalTube.Refresh = refresh_display;
    assert(Tx.huart == &huart2 && Rx.Handle.huart == &huart2 && Rx.Mesg_Len == 14);
}
static void scan_at(uint32_t elapsed)
{
    tick = base + elapsed;
    Key_Task();
}
static void joystick_at(uint32_t elapsed, uint16_t pressed)
{
    /* 使用原理图中的实际 PA4～PA7 引脚，独立检查固件引脚映射。 */
    test_gpio_a.IDR = (test_gpio_a.IDR | 0x00f0u) & ~pressed;
    scan_at(elapsed);
}
static void expect_frame(unsigned index, uint8_t code, uint8_t value, uint8_t event)
{
    assert(index < frame_count);
    const uint8_t *frame = frames[index];
    assert(frame[0] == 0xaa && frame[1] == 0 && frame[3] == 3);
    assert(frame[4] == code && frame[5] == 0 && frame[6] == 0 && frame[7] == 0);
    assert(frame[8] == value && frame[9] == 0 && frame[10] == event && frame[13] == 0x55);
    assert((((uint16_t)frame[11] << 8) | frame[12]) == crc16(frame, 11));
    if (index > 0)
        assert(frame[2] == (uint8_t)(frames[index - 1][2] + 1));
}
static void test_eight_directions(void)
{
    /* 上、下、左、右、左上、左下、右上、右下。 */
    const uint16_t pins[] = {0x20, 0x10, 0x80, 0x40, 0xa0, 0x90, 0x60, 0x50};
    for (unsigned i = 0; i < 8; i++)
    {
        reset_test(0);
        joystick_at(10, pins[i]);
        scan_at(24);
        assert(frame_count == 0);
        scan_at(25);
        assert(frame_count == 0); // 沿用短按在释放时判定的时序
        joystick_at(110, 0);
        scan_at(124);
        assert(frame_count == 0);
        scan_at(125);
        assert(frame_count == 2);
        expect_frame(0, 4, i + 1, 1);
        expect_frame(1, 4, i + 1, 3);
        if (i == 0)
        {
            /* 固定首帧向量同时验证字节顺序、ID 与 CRC 的高低字节。 */
            const uint8_t golden[] = {0xaa, 0, 1, 3, 4, 0, 0, 0, 1, 0, 1, 0x7b, 0xbd, 0x55};
            assert(memcmp(frames[0], golden, sizeof(golden)) == 0);
        }
        scan_at(2000);
        assert(frame_count == 2); // 释放后不继续重复上报
    }
}
static void test_long_press(uint32_t start)
{
    reset_test(start);
    joystick_at(10, 0x60); // 右上
    scan_at(25);
    scan_at(809);
    assert(frame_count == 0);
    scan_at(810);
    assert(frame_count == 1);
    expect_frame(0, 4, 7, 2);
    scan_at(1142);
    assert(frame_count == 1);
    scan_at(1143);
    assert(frame_count == 2);
    expect_frame(1, 4, 7, 2);
    joystick_at(1150, 0);
    scan_at(1165);
    assert(frame_count == 3);
    expect_frame(2, 4, 7, 3); // 长按后松开不补发短按
}
static void test_bounce_and_diagonal(void)
{
    reset_test(0);
    joystick_at(10, 0x20);
    joystick_at(20, 0);
    scan_at(40);
    assert(frame_count == 0); // 小于 15ms 的噪声不产生事件
    joystick_at(60, 0x20);
    joystick_at(70, 0xa0); // 两路相隔 10ms 闭合，合并为左上
    scan_at(85);
    joystick_at(100, 0x20); // 释放抖动不足 15ms 后恢复
    joystick_at(110, 0xa0);
    scan_at(130);
    assert(frame_count == 0);
    joystick_at(200, 0);
    scan_at(215);
    assert(frame_count == 2);
    expect_frame(0, 4, 5, 1);
    expect_frame(1, 4, 5, 3);
}
static void test_direction_changes(void)
{
    reset_test(0);
    joystick_at(10, 0x20);
    scan_at(25);
    joystick_at(200, 0xa0);
    scan_at(215);
    joystick_at(400, 0x80);
    scan_at(415);
    joystick_at(600, 0);
    scan_at(615);
    assert(frame_count == 6);
    expect_frame(0, 4, 1, 1);
    expect_frame(1, 4, 1, 3);
    expect_frame(2, 4, 5, 1);
    expect_frame(3, 4, 5, 3);
    expect_frame(4, 4, 3, 1);
    expect_frame(5, 4, 3, 3);
    reset_test(0);
    joystick_at(10, 0x20);
    scan_at(25);
    scan_at(810);
    joystick_at(900, 0x10); // 长按后反向，只释放旧方向
    scan_at(915);
    joystick_at(1000, 0);
    scan_at(1015);
    assert(frame_count == 4);
    expect_frame(0, 4, 1, 2);
    expect_frame(1, 4, 1, 3);
    expect_frame(2, 4, 2, 1);
    expect_frame(3, 4, 2, 3);
}
static void test_invalid_and_delayed_scan(void)
{
    const uint16_t invalid[] = {0, 0x30, 0xc0, 0x70, 0xb0, 0xd0, 0xe0, 0xf0};
    for (unsigned i = 0; i < sizeof(invalid) / sizeof(invalid[0]); i++)
    {
        reset_test(0);
        joystick_at(10, invalid[i]);
        scan_at(25);
        scan_at(2000);
        assert(frame_count == 0);
    }
    reset_test(0);
    joystick_at(10, 0x20);
    scan_at(25);
    joystick_at(100, 0x30); // 上下同时触发时释放旧方向
    scan_at(115);
    assert(frame_count == 2);
    expect_frame(0, 4, 1, 1);
    expect_frame(1, 4, 1, 3);
    reset_test(0);
    joystick_at(10, 0x10);
    scan_at(25);
    joystick_at(1000, 0); // 主循环延迟，仍按实际持续时间判为长按
    scan_at(1015);
    assert(frame_count == 2);
    expect_frame(0, 4, 2, 2);
    expect_frame(1, 4, 2, 3);
}
static void test_buttons(void)
{
    const uint16_t pins[] = {0x400, 0x800, 0x1000, 0x20, 0x40, 0x80, 0x100, 0x200};
    for (unsigned i = 0; i < 8; i++)
    {
        reset_test(0);
        GPIO_TypeDef *gpio = i < 3 ? GPIOA : GPIOB;
        gpio->IDR &= ~pins[i];
        scan_at(10);
        scan_at(40);
        gpio->IDR |= pins[i];
        scan_at(100);
        assert(frame_count == 1);
        expect_frame(0, i < 6 ? 1 : 2, i < 6 ? i + 1 : i - 5, 1);
    }
    reset_test(0);
    test_gpio_a.IDR &= ~0x400u;
    scan_at(10);
    scan_at(810);
    assert(frame_count == 1);
    expect_frame(0, 1, 1, 2);
}

static void test_concurrent_inputs(void)
{
    /* 普通按键与摇杆可同时保持，各自计时且使用不同命令码。 */
    reset_test(0);
    test_gpio_a.IDR &= ~0x400u;
    joystick_at(10, 0x20);
    scan_at(25);
    scan_at(810);
    assert(frame_count == 2);
    expect_frame(0, 1, 1, 2);
    expect_frame(1, 4, 1, 2);
    test_gpio_a.IDR |= 0x400u;
    joystick_at(900, 0);
    scan_at(915);
    assert(frame_count == 3);
    expect_frame(2, 4, 1, 3);
}

/* 经真实 UART 回调与环形缓冲区投递，检查分段帧、坏帧和粘包。 */
static void receive_bytes(const uint8_t *data, unsigned count)
{
    for (unsigned i = 0; i < count; i++)
    {
        *receive_byte = data[i];
        HAL_UART_RxCpltCallback(&huart2);
    }
}
static void make_command(uint8_t *frame, uint8_t code, uint32_t value)
{
    const uint8_t header[] = {0xaa, 0, 0x21, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x55};
    memcpy(frame, header, 14);
    frame[4] = code;
    frame[5] = value >> 24;
    frame[6] = value >> 16;
    frame[7] = value >> 8;
    frame[8] = value;
    uint16_t crc = crc16(frame, 11);
    frame[11] = crc >> 8;
    frame[12] = crc;
}
static void test_downlink(void)
{
    reset_test(0);
    uint8_t frame[14];
    make_command(frame, 1, 0xaa55aa55u);
    receive_bytes(frame, 7);
    CommTask();
    assert(display_count == 0);
    receive_bytes(frame + 7, 7);
    CommTask();
    assert(display_count == 1 && refresh_count == 1 && display_value == 0xaa55aa55u);
    for (unsigned code = 2; code <= 5; code++)
    {
        make_command(frame, code, 0xffffffffu);
        receive_bytes(frame, 14);
    }
    make_command(frame, 1, 1234);
    receive_bytes(frame, 14);
    CommTask();
    assert(display_count == 2 && refresh_count == 2 && display_value == 1234);
    make_command(frame, 1, 9876);
    frame[11] ^= 1;
    receive_bytes(frame, 14);
    make_command(frame, 1, 5678);
    receive_bytes(frame, 14);
    CommTask();
    assert(display_count == 3 && display_value == 5678);
    receive_bytes(frame, 5); // 截断帧后紧接完整帧，必须恢复同步
    receive_bytes(frame, 14);
    CommTask();
    assert(display_count == 4 && display_value == 5678);
    frame[3] = 3; // 控台上行帧不能作为主板下行执行
    uint16_t crc = crc16(frame, 11);
    frame[11] = crc >> 8;
    frame[12] = crc;
    receive_bytes(frame, 14);
    CommTask();
    assert(display_count == 4 && frame_count == 0);
}
static void test_id_wrap(void)
{
    reset_test(0);
    for (unsigned i = 0; i < 260; i++)
    {
        Comm_SendMesg_FillData(&Tx, Ctrl_to_Board, 4, 8, 3);
        expect_frame(i, 4, 8, 3);
    }
}
int main(void)
{
    assert(sizeof(Mesg_TypeDef) == 14);
    assert(crc16((const uint8_t *)"123456789", 9) == 0x4b37);
    test_eight_directions();
    test_long_press(0);
    test_long_press(UINT32_MAX - 1000u);
    test_bounce_and_diagonal();
    test_direction_changes();
    test_invalid_and_delayed_scan();
    test_buttons();
    test_concurrent_inputs();
    test_downlink();
    test_id_wrap();
    puts("通过：八方向、消抖、长按重复、松开、转向、非法组合、时钟/消息ID回绕、全部按键、CRC、下行与坏帧恢复");
    return 0;
}
