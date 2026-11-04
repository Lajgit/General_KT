#include "CommTask.h"
#include "app_crc.h"
#include "port_digitaltube.h"
#include "string.h"
#include "usart.h"

#define Mesg_Head 0xAA
#define Mesg_Tail 0x55

static uint8_t rx_buffer[256];
static Mesg_TypeDef Receive_mesg;

Tx_HandleTypeDef Tx;
Rx_HandleTypeDef Rx;

extern DigitalTube_t DigitalTube;

/// 串口消息验证
static bool USART_ReceiveMesg_Verify(void *self, void *mesg)
{
    Rx_HandleTypeDef *rx = (Rx_HandleTypeDef *)self;
    Mesg_TypeDef *Rx_mesg = (Mesg_TypeDef *)mesg;
    uint16_t crc16, mesg_crc16;
    crc16 = CRC16_calculate(rx->Queue.Buf, 11);
    mesg_crc16 = Rx_mesg->CRC16_H << 8 | Rx_mesg->CRC16_L;
    if (crc16 == mesg_crc16)
        return true;
    return false;
}
/// USART2 消息处理
static void USART_Deal(void *Rx_mesg)
{
    Mesg_TypeDef *mesg = (Mesg_TypeDef *)Rx_mesg;
    if (mesg->Code1 == Board_to_Ctrl)
    {
        switch (mesg->Code2)
        {
        /// 数码管
        case 0x01:
        {
            /* 先转为无符号数再移位，支持完整 32 位下发值。 */
            uint32_t data = (uint32_t)mesg->Data1 << 24 | (uint32_t)mesg->Data2 << 16
                            | (uint32_t)mesg->Data3 << 8 | mesg->Data4;
            DigitalTube.Set_Num(&DigitalTube, 0, data, 4);
            DigitalTube.Refresh(&DigitalTube);
            break;
        }
        /* 通用控台没有 RGB 灯，旧灯光指令统一忽略。 */
        default:
            break;
        }
    }
}

///====================================================================================

/// 发送消息，无重传
static uint8_t USART_SendMesg(Tx_HandleTypeDef *Tx, Mesg_TypeDef *mesg)
{
    static uint8_t ID = 0;
    uint8_t data[14];
    uint16_t crc;

    ID++;               // 每次发送新消息都会自增
    mesg->ResendID = 0; // 重发次数清零
    mesg->ID = ID;      // 赋予新ID号
    memcpy(data, mesg, 14);
    crc = CRC16_calculate(data, 11);
    data[11] = crc >> 8;
    data[12] = crc;
    // Tx->Transimit(&Tx, data, 14);
    HAL_UART_Transmit(Tx->huart, data, 14, 100);
    return ID;
}
/// 填入参数发送消息，无重传
uint8_t Comm_SendMesg_FillData(Tx_HandleTypeDef *Tx, uint8_t code_1, uint8_t code_2, uint32_t data, uint8_t expandCode)
{
    Mesg_TypeDef mesg = {0};
    mesg.Head = Mesg_Head;
    mesg.ResendID = 0;
    mesg.ID = 0;
    mesg.Code1 = code_1;
    mesg.Code2 = code_2;
    mesg.Data1 = (uint8_t)(data >> 24);
    mesg.Data2 = (uint8_t)(data >> 16);
    mesg.Data3 = (uint8_t)(data >> 8);
    mesg.Data4 = (uint8_t)(data);
    mesg.ACKbyte = 0x00;
    mesg.ExpandCode = expandCode;
    mesg.Tail = Mesg_Tail;
    return USART_SendMesg(Tx, &mesg);
}

/*
 * ----------通信初始化----------
 */
void CommInit(void)
{

    Rx_InitTypeDef Rxinit = {0};

    Rxinit.huart = &huart2;
    Rxinit.RingBuf = rx_buffer;
    Rxinit.RingBuf_Size = sizeof(rx_buffer);
    Rxinit.Mesg_Len = sizeof(Mesg_TypeDef); // 固定 14 字节协议帧
    Rxinit.Frame_Head = Mesg_Head;
    Rxinit.Frame_Tail = Mesg_Tail;
    Rxinit.Receive = Rx_Receive;
    Rxinit.Verify = USART_ReceiveMesg_Verify;
    Rxinit.Deal = USART_Deal;
    Communicate_Rx_Init(&Rx, Rxinit);

    Tx_InitTypeDef Tx_init;
    Tx_init.huart = &huart2;
    Tx_init.hdma = NULL;
    Tx_init.TxBuf = NULL;
    Tx_init.TxBuf_Size = 0;
    Communicate_Tx_Init(&Tx, Tx_init);
}

void CommTask(void)
{
    Rx.Receive(&Rx, &Receive_mesg, 14);
}
