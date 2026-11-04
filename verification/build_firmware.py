#!/usr/bin/env python3
"""读取 Keil 工程中的源文件和宏，用 GNU Arm 工具链编译并链接验证。"""
import argparse
from pathlib import Path
import subprocess
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--toolchain', default='', help='arm-none-eabi-gcc 所在目录')
    args = parser.parse_args()
    prefix = str(Path(args.toolchain) / 'arm-none-eabi-') if args.toolchain else 'arm-none-eabi-'
    project = ROOT / 'MDK-ARM/General_KT.uvprojx'
    tree = ET.parse(project)
    output = ROOT / 'build/firmware'
    output.mkdir(parents=True, exist_ok=True)
    flags = ['-mcpu=cortex-m3', '-mthumb', '-std=c99', '-Os', '-g3',
             '-ffunction-sections', '-fdata-sections', '-Wall', '-Wextra', '-Wno-unused-parameter']
    # 直接使用现有 Keil 清单，确保新增文件也进入实际工程。
    for value in tree.find('.//Cads/VariousControls/Define').text.split(','):
        flags.append('-D' + value)
    for value in tree.find('.//Cads/VariousControls/IncludePath').text.split(';'):
        flags += ['-I', str((project.parent / value.replace('\\', '/')).resolve())]
    sources = [(project.parent / item.text.replace('\\', '/')).resolve()
               for item in tree.findall('.//Groups/Group/Files/File/FilePath')
               if item.text.endswith('.c')]
    sources.append(ROOT / 'Drivers/CMSIS/Device/ST/STM32F1xx/Source/Templates/gcc/startup_stm32f103xb.s')
    sources.append(ROOT / 'verification/gcc_runtime.c')
    objects = []
    for index, source in enumerate(sources):
        obj = output / f'{index:02d}_{source.stem}.o'
        subprocess.run([prefix + 'gcc', *flags, '-c', str(source), '-o', str(obj)], check=True)
        objects.append(str(obj))
    elf = output / 'General_KT.elf'
    subprocess.run([prefix + 'gcc', *flags, *objects, '--specs=nano.specs', '--specs=nosys.specs',
                    '-nostartfiles', '-T', str(ROOT / 'verification/firmware.ld'),
                    '-Wl,--gc-sections', '-Wl,-Map=' + str(output / 'General_KT.map'),
                    '-Wl,--print-memory-usage', '-o', str(elf)], check=True)
    for fmt, suffix in [('ihex', 'hex'), ('binary', 'bin')]:
        subprocess.run([prefix + 'objcopy', '-O', fmt, str(elf), str(elf.with_suffix('.' + suffix))], check=True)
    subprocess.run([prefix + 'size', str(elf)], check=True)
    print(f'已编译并链接 {len(sources)} 个源文件：{elf}')


if __name__ == '__main__':
    main()
