#include "port_communicate.h"
#include <string.h>

/*
 * ================================TX=========================================
 */

/*
 * @brief 发送数据函数
 * @param handle Tx_HandleTypeDef 结构体指针
 * @param data 待发送数据指针
 * @param len 待发送数据长度
 */
static void Communicate_Tx_Transmit(void *handle, uint8_t *data, uint16_t len)
{
    Tx_HandleTypeDef *Handle = (Tx_HandleTypeDef *)handle;
    if (Handle->huart == NULL || data == NULL || len == 0)
        return;
    HAL_UART_Transmit(Handle->huart, data, len, 100);
}

/*
 * @brief 初始化串口发送模块
 * @param handle Tx_HandleTypeDef 句柄指针
 * @param Tx_Init Tx_InitTypeDef 初始化参数
 */
void Communicate_Tx_Init(Tx_HandleTypeDef *Handle, Tx_InitTypeDef Tx_Init)
{
    Handle->huart = Tx_Init.huart;
    Handle->hdma = Tx_Init.hdma;
    Handle->TxBuf = Tx_Init.TxBuf;
    Handle->TxBuf_Size = Tx_Init.TxBuf_Size;
    Handle->Transimit = Communicate_Tx_Transmit;
    Handle->huart->gState = HAL_UART_STATE_READY;
}

/*
 * ================================RX=========================================
 */

/* 固定长度解帧，数据区或 CRC 中的 0xAA/0x55 不作为分隔符。 */
void Rx_Receive(void *self, void *mesg, uint8_t mesg_len)
{
    Rx_HandleTypeDef *rx = (Rx_HandleTypeDef *)self;
    if (mesg == NULL || mesg_len < 2 || mesg_len > rx->Queue.Buf_Size)
        return;

    while (rx->Handle.RingBuf.f_ReadByte(&rx->Handle.RingBuf, &rx->CurrData))
    {
        if (rx->State == WAIT_HEAD)
        {
            if (rx->CurrData != rx->Frame_Head)
                continue;
            rx->Queue.Index = 0;
            rx->State = RECEIVE_DATA;
        }
        rx->Queue.Buf[rx->Queue.Index++] = rx->CurrData;
        if (rx->Queue.Index != mesg_len)
            continue;

        /* 收满一帧再验证，旧灯光帧也完整消费，避免影响下一帧。 */
        memcpy(mesg, rx->Queue.Buf, mesg_len);
        if (rx->Queue.Buf[mesg_len - 1] == rx->Frame_Tail &&
            (rx->Verify == NULL || rx->Verify(rx, mesg)))
        {
            rx->State = WAIT_HEAD;
            rx->Queue.Index = 0;
            if (rx->Deal != NULL)
                rx->Deal(mesg);
        }
        else
        {
            /* 丢字节或 CRC 错误时，从缓冲区内下一个帧头重新同步。 */
            uint8_t next = 1;
            while (next < mesg_len && rx->Queue.Buf[next] != rx->Frame_Head)
                next++;
            rx->Queue.Index = mesg_len - next;
            memmove(rx->Queue.Buf, rx->Queue.Buf + next, rx->Queue.Index);
            rx->State = rx->Queue.Index == 0 ? WAIT_HEAD : RECEIVE_DATA;
        }
    }
}

/*
 * @brief 初始化串口接收模块
 * @param handle Rx_HandleTypeDef 句柄指针
 * @param Rx_Init Rx_InitTypeDef 初始化参数
 */
void Communicate_Rx_Init(Rx_HandleTypeDef *rx, Rx_InitTypeDef Rx_Init)
{
    rx->Handle.huart = Rx_Init.huart;
    RingBuffer_Init(&rx->Handle.RingBuf, Rx_Init.RingBuf, Rx_Init.RingBuf_Size);
    rx->CurrData = 0;
    rx->State = WAIT_HEAD;
    rx->Queue.Buf_Size = Queue_Size;
    rx->Frame_Head = Rx_Init.Frame_Head;
    rx->Frame_Tail = Rx_Init.Frame_Tail;
    rx->Mesg_Len = Rx_Init.Mesg_Len;
    if (Rx_Init.Receive == NULL)
        rx->Receive = Rx_Receive;
    else
        rx->Receive = Rx_Init.Receive;
    rx->Verify = Rx_Init.Verify;
    rx->Deal = Rx_Init.Deal;

    HAL_UART_Receive_IT(rx->Handle.huart, &rx->Handle.temp_data, 1);
}
