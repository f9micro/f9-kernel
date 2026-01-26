# Quick Start Guide

## Supported Boards

F9 Microkernel supports the following boards:
* [STM32F4DISCOVERY](https://www.st.com/en/evaluation-tools/stm32f4discovery.html)
* [STM32F429I-DISC1](https://www.st.com/en/evaluation-tools/32f429idiscovery.html)
* [NUCLEO-F429ZI](https://www.st.com/en/evaluation-tools/nucleo-f429zi.html)
  - All supported boards are based on the ARM Cortex-M4F core. F9 should work on any STM32F40x/STM32F429/STM32F439 microcontroller.
* Netduino Plus 2 (STM32F405RGT6)
  - Supported by upstream [QEMU for emulation](https://www.qemu.org/docs/master/system/arm/stm32.html), making it ideal for development and testing without hardware.

## Toolchain Requirements

Building F9 Microkernel requires an arm-none-eabi toolchain with Cortex-M4F support:
* [Arm GNU Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads)
  - Download the AArch32 bare-metal target (`arm-none-eabi`) package.

## Building F9

### Configuration

Configuration is the first build step. Run `make config` to select options.
The build system generates a `.config` file and a configuration header
`include/autoconf.h` for C programs.

### Compilation

Run `make` to build. Output files are placed in the `build` directory.

## Flashing and Debugging

### STM32F4 Boards

For flashing and debugging on STM32F4 boards, [stlink](https://github.com/stlink-org/stlink)
is required. With `st-flash` in the PATH, `make flash` programs the
STM32F4DISCOVERY board with the built F9 binary image.

Source-level debugging with gdb is possible via OpenOCD or stlink, but may be
limited on resource-constrained targets.

### In-Kernel Debugger (KDB)

KDB (in-kernel debugger) is enabled by default and provides essential kernel inspection. Press `?` to display the menu:

| Key | Function                        |
|-----|---------------------------------|
| `a` | Dump address spaces             |
| `e` | Dump ktimer events              |
| `K` | Print kernel tables             |
| `m` | Dump memory pools               |
| `M` | Dump MPU status                 |
| `n` | Show timer (now)                |
| `p` | Show sampling (requires SYMMAP) |
| `s` | Show softirqs                   |
| `t` | Dump threads                    |
| `v` | Show tickless scheduling status |

KDB communicates via USART. The port can be selected during `make config`.

### Serial Port Configuration

#### STM32F4DISCOVERY

| Port   | TX Pin | RX Pin |
|--------|--------|--------|
| USART4 | PA0    | PA1    |
| USART2 | PA2    | PA3    |
| USART1 | PA9    | PA10   |

#### STM32F429I-DISC1

| Port   | TX Pin | RX Pin |
|--------|--------|--------|
| USART4 | PC11   | PC10   |
| USART2 | PD5    | PD6    |
| USART1 | PA9    | PA10   |

#### NUCLEO-F429ZI

| Port   | TX Pin | RX Pin |
|--------|--------|--------|
| USART3 | PD8    | PD9    |
| USART2 | PD5    | PD6    |
| USART1 | PB6    | PB7    |

> Note: On the NUCLEO-F429ZI, `USART3` is connected to the on-board
> VCOM (Virtual COM Port) provided by the ST-LINK.

## QEMU Emulation

For Netduino Plus 2 under QEMU, the default configuration uses USART1, which
QEMU routes to the console. Run with:

```shell
qemu-system-arm -M netduinoplus2 -nographic -serial mon:stdio \
    -kernel build/netduinoplus2/f9.elf
```

Note: `-serial mon:stdio` is required for interactive KDB shell.
The `mon:` prefix enables QEMU monitor access via `Ctrl+a` and `c`. Exit with `Ctrl+a` and `x`.

## Serial Terminal Setup

For physical boards, connect a USB-to-serial adapter and use a terminal emulator
at 115200 baud, 8N1. For example, with `screen`:

```shell
screen /dev/ttyUSB0 115200 8n1
```

Press `Ctrl-a` and `k to exit screen.
