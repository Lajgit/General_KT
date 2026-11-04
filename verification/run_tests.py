#!/usr/bin/env python3
"""在主机上编译真实控台业务源代码，通过模拟 GPIO 和 UART 运行集成测试。"""
from pathlib import Path
import subprocess
import os

ROOT = Path(__file__).resolve().parents[1]
output = ROOT / 'build/tests'
output.mkdir(parents=True, exist_ok=True)
sources = ['verification/test_control.c', 'Task/Src/KeyTask.c', 'Task/Src/CommTask.c',
           'Task/Src/InterruptTask.c', 'Mid/Src/port_key.c', 'Mid/Src/port_joystick.c',
           'Mid/Src/port_communicate.c', 'Lib/Src/app_crc.c', 'Lib/Src/app_ringbuf.c']
flags = ['-std=c99', '-g', '-O1', '-Wall', '-Wextra', '-Werror', '-Wno-unused-parameter',
         '-fsanitize=address,undefined', '-fno-omit-frame-pointer']
for directory in ['verification/stubs', 'Core/Inc', 'Task/Inc', 'Mid/Inc', 'Lib/Inc']:
    flags += ['-I', str(ROOT / directory)]
binary = output / 'test_control'
subprocess.run([os.environ.get('CC', 'gcc'), *flags, *[str(ROOT / s) for s in sources],
                '-o', str(binary)], check=True)
subprocess.run([str(binary)], check=True)
