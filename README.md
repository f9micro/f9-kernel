# F9 Microkernel

F9 is an experimental microkernel for building flexible embedded systems,
inspired by the [L4 microkernel family](https://en.wikipedia.org/wiki/L4_microkernel_family).
It aims to bring modern kernel techniques to ARM Cortex-M microprocessors,
enabling real-time and time-sharing applications (such as wireless communications)
with a focus on efficiency (performance and power consumption) and security
(memory protection and isolated execution).

## Characteristics
* F9 follows the fundamental principles of microkernels in that it implements
  address spaces, thread management, and IPC only in the privileged kernel.
* Designed and customized for ARM Cortex-M, supporting NVIC (Nested Vectored
  Interrupt Controller), Bit Banding, MPU (Memory Protection Unit).
* Energy-efficient scheduling and tickless timer allow the ARM Cortex-M to
  wake up only when needed, either at a scheduled time or on an interrupt event.
  This results in lower power consumption compared to the traditional SysTick
  approach, which requires a constantly running high-frequency clock.
* KProbes, a dynamic instrumentation system inspired by the Linux kernel, allowing
  developers to gather additional information about kernel operation without
  recompiling or rebooting the kernel. It enables locations in the kernel to
  be instrumented with code, and the instrumentation code runs when the ARM
  core encounters that probe point. Once the instrumentation code completes
  execution, the kernel continues normal execution.
* Each thread has its own TCB (Thread Control Block) and is addressed by its
  global ID. The dispatcher is responsible for context switching. Threads
  with the same priority are scheduled in round-robin fashion.
* Memory management is split into three concepts:
  - Memory pool: an area of physical address space with specific attributes.
  - Flexible page: a size-aligned region of an address space. Unlike other L4
    implementations, flexible pages in F9 represent MPU regions.
  - Address space: composed of flexible pages.
* System calls are provided to manage address spaces:
  - Grant: The memory page is granted to a new user and cannot be used anymore
    by its former user.
  - Map: This implements shared memory – the memory page is passed to another
    task but can be used by both tasks.
  - Flush: The memory page that has been mapped to other users will be flushed
    out of their address space.
* User threads interact with the microkernel through UTCBs (User-level Thread
  Control Blocks). A UTCB is a small thread-specific region in the thread's
  virtual address space that is always mapped. Since UTCB access never raises
  a page fault, it is ideal for the kernel to access system call arguments,
  particularly IPC payload copied from/to user threads.
* The kernel provides synchronous IPC (inter-process communication). Short IPC
  carries payload in CPU registers only, while full IPC copies message payload
  via the UTCBs of the communicating parties.
* Debugging and profiling mechanisms:
  - configurable debug console
  - memory dump
  - thread profiling: name, uptime, stack allocated/current/used
  - memory profiling: kernel table, pool free/allocated size, fragmentation

## Quick Start
F9 Microkernel supports the following boards:
* [STM32F4DISCOVERY](https://www.st.com/en/evaluation-tools/stm32f4discovery.html)
* [STM32F429I-DISC1](https://www.st.com/en/evaluation-tools/32f429idiscovery.html)
  - Both are based on ARM Cortex-M4F core. F9 should work on any STM32F40x/STM32F429/STM32F439 microcontroller.
* Netduino Plus 2 (STM32F405RGT6)
  - Supported by upstream [QEMU for emulation](https://www.qemu.org/docs/master/system/arm/stm32.html), making it ideal for development and testing without hardware.

Building F9 Microkernel requires an arm-none-eabi toolchain with Cortex-M4F support:
* [Arm GNU Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads)
  - Download the AArch32 bare-metal target (arm-none-eabi) package.

Configuration is the first build step. Run `make config` to select options.
The build system generates a `.config` file and a configuration header
`include/autoconf.h` for C programs.

Run `make` to build. Output files are placed in the `build` directory.

For flashing and debugging on STM32F4 boards, [stlink](https://github.com/stlink-org/stlink)
is required. With `st-flash` in the PATH, `make flash` programs the
STM32F4DISCOVERY board with the built F9 binary image.

Source-level debugging with gdb is possible via OpenOCD or stlink, but may be
limited on resource-constrained targets. KDB (in-kernel debugger) is enabled by
default and provides essential kernel inspection. Press `?` to display the menu:

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

For STM32F4DISCOVERY:

| Port   | TX Pin | RX Pin |
|--------|--------|--------|
| USART4 | PA0    | PA1    |
| USART2 | PA2    | PA3    |
| USART1 | PA9    | PA10   |

For STM32F429I-DISC1:

| Port   | TX Pin | RX Pin |
|--------|--------|--------|
| USART4 | PC11   | PC10   |
| USART2 | PD5    | PD6    |
| USART1 | PA9    | PA10   |

For Netduino Plus 2 under QEMU, the default configuration uses USART1, which
QEMU routes to the console. Run with:

    qemu-system-arm -M netduinoplus2 -nographic -serial mon:stdio \
        -kernel build/netduinoplus2/f9.elf

For physical boards, connect a USB-to-serial adapter and use a terminal emulator
at 115200 baud, 8N1. For example, with `screen`:

    screen /dev/ttyUSB0 115200 8n1

Press Ctrl-a k to exit screen.

## Build Configurations
F9 Microkernel uses a Linux kernel-style build system (Kconfig/Kconfiglib).
The key build files are:

* toolchain.mk:
  - toolchain-specific configurations; common cflags and ldflags
* platform/build.mk:
  - platform-specific configurations; cflags/ldflags for CPU and FPU
* board/`<BOARD_NAME>`/build.mk:
  - board-specific configurations; CHIP model, peripherals
* rules.mk: the magic of build system

Board-specific resource assignments can be modified in `board/<BOARD_NAME>/board.[ch]`.
The configuration header `include/autoconf.h` documents the available options:

* `CONFIG_DEBUG`
  - Enable serial I/O for debugging. An additional service for serial character
    operations will be included.
* `CONFIG_KDB`
  - Enable in-kernel debugger.
* `CONFIG_KPROBES`
  - Enable KProbes, the lightweight dynamic instrumentation system.
* `CONFIG_SYMMAP`
  - Generate symbol map for KProbes and profiling support.
* `CONFIG_KTIMER_TICKLESS`
  - Enable tickless scheduling for improved power efficiency.
* `CONFIG_KTIMER_TICKLESS_VERIFY`
  - Track and verify tickless scheduling status (depends on KTIMER_TICKLESS).
* `CONFIG_MAX_`xxx series
  - Limits for threads, ktimer events, async events, address spaces, and
    flexible pages.
* `CONFIG_PANIC_DUMP_STACK`
  - Dump kernel stack on panic.
* `CONFIG_QEMU`
  - Enable workarounds for QEMU emulation (auto-enabled for Netduino Plus 2).

## Licensing
F9 Microkernel is freely redistributable under the two-clause BSD License.
Use of this source code is governed by a BSD-style license that can be found
in the `LICENSE` file.
