# 通用控台小卡 General_KT

按《通用控台小卡.pdf》适配 APM32F103C8T6，沿用参考工程的 STM32F1 HAL 驱动与 Keil/EIDE 构建方式。参考版本为 PANTAO_WIFI/PanTao_Ctrl 与 DANJIE_WIFI/Danjie_Ctrl。

## 硬件连接

| 功能 | 引脚/接口 |
| --- | --- |
| 状态指示灯（低电平点亮） | PA0 |
| 主板通信 | USART2：PA2 TX、PA3 RX，115200、8N1 |
| 旋钮 | TIM1：PA8 CW、PA9 CCW |
| 摇杆上、下、左、右 | PA5、PA4、PA7、PA6，低电平有效 |
| 拍拍按键 1～6 | PA10、PA11、PA12、PB5、PB6、PB7，低电平有效 |
| 扩展按键 1、2 | PB8、PB9，低电平有效 |
| 数码管 J4 | PB15 SER、PB14 OE、PB13 SRCLK、PB12 RCLK |
| 调试接口 | PA13 SWDIO、PA14 SWCLK |

原理图没有旋钮按压开关和 RGB 灯。六路拍拍按键沿用 `Code2=0x01`、编号 1～6；两路扩展按键沿用原键盘 `Code2=0x02`、编号 1～2；旋钮沿用 `Code2=0x03`，正向计数事件为 `ExpandCode=0x00`，反向为 `0x01`。短按在释放时判定，拍拍按键长按门限为 800ms，长按重复频率为 3Hz，沿用原按键处理方式。

主板下发只保留 `Code1=0x02, Code2=0x01` 数码管数据。原有灯带亮度 `0x02`、灯光场景 `0x03`、控台亮度 `0x04`、控台按键灯 `0x05` 均忽略；RGB 任务、PWM、TIM3 与相关 DMA 已移除。PA0 状态指示灯保留。

## 编译验证

Windows 可使用 `MDK-ARM/General_KT.uvprojx`，或 EIDE 工程。Linux 安装 GNU Arm Embedded 工具链后执行：

```sh
python3 verification/build_firmware.py --toolchain /path/to/arm-gcc/bin
```

脚本读取 Keil 项目中的实际 C 源文件、宏和包含路径，用 Cortex-M3 配置完整编译并链接到 64KB Flash、20KB SRAM，输出 `build/firmware/General_KT.elf/.hex/.bin`。构建产物不进入 Git。GNU Arm 编译用于验证，尚需在实板上验证 APM32 的串口、编码器物理方向和数码管段码。
