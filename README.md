F9 Microkernel
==============

This is `F9`, an experimental microkernel used to construct flexible embedded
systems inspired by famous [L4 microkernel](http://en.wikipedia.org/wiki/L4_microkernel_family).
The motivation of F9 microkernel is to deploy modern kernel techniques to
support running real-time and time-sharing applications (for example, wireless
communications) for ARM Cortex-M series microprocessors with efficiency
(performanace + power consumption) and security (memory protection + isolated
execution) in mind.


Characteristics of F9 Microkernel
=================================

* F9 follows the fundamental principles of microkernels in that it implements
  address spaces, thread management, and IPC only in the privileged kernel.

* Designed and customized for ARM Cortex-M, supporting NVIC (Nested Vectored
  Interrupt Controller), Bit Banding, MPU (Memory Protection Unit).

* Energy efficient scheduling and tickless timer which allow the ARM Cortex-M
  to wake up only when needed, either at a scheduled time or on an interrupt
  event. Therefore, it results in better current consumption than the common
  approach using the system timer, SysTick, which requires a constantly
  running and high frequency clock.

* KProbes, dynamic instrumentation system inspired by Linux Kernel, allowing
  developers to gather additional information about kernel operation without
  recompiling or rebooting the kernel. It enables locations in the kernel to
  be instrumented with code, and the instrumentation code runs when the ARM
  core encounters that probe point. Once the instrumentation code completes
  execution, the kernel continues normal execution.

* Each thread has its own TCB (Thread Control Block) and addressed by its
  global id. Also dispatcher is responsible for switching contexts. Threads
  with the same priority are executed in a round-robin fashion.

* Memory management is split into three concepts:
  - Memory pool, which represent area of physical address space with specific
    attributes.
  - Flexible page, which describes an always size aligned region of an address
    space. Unlike other L4 implementations, flexible pages in F9 represent MPU
    region instead.
  - Address space, which is made up of these flexible pages.

* System calls are provided to manage address spaces:
  - Grant: The memory page is granted to a new user and cannot be used anymore
    by its former user.
  - Map: This implements shared memory – the memory page is passed to another
    task but can be used by both tasks.
  - Flush: The memory page that has been mapped to other users will be flushed
    out of their address space.

* Regarding the interaction between a user thread and the microkernel, the
  concept of UTCB (user-level thread-control blocks) is being taken on. A UTCB
  is a small thread-specific region in the thread's virtual address space,
  which is always mapped. Therefore, the access to the UTCB can never raise a
  page fault, which makes it perfect for the kernel to access system-call
  arguments, in particular IPC payload copied from/to user threads.

* The kernel provides synchronous IPC (inter-process communication), for which
  short IPC carries payload in CPU registers only and full IPC copies message
  payload via the UTCBs of the communicating parties.

* Debugging and profiling mechanisms:
  - configurable debug console
  - memory dump
  - thread profiling: name, uptime, stack allocated/current/used
  - memory profiling: kernel table, pool free/allocated size, fragmentation


Licensing
=========

F9 Microkernel is freely redistributable under the two-clause BSD License.
Use of this source code is governed by a BSD-style license that can be found
in the `LICENSE` file.


Quick Start
===========

The current hardware board F9 Microkernel supports is [STM32F4DISCOVERY](http://www.st.com/web/en/catalog/tools/FM116/SC959/SS1532/PF252419)
based on ARM Cortex-M4F core, but F9 should work well on any STM32F40x
microcontroller.

Building F9 Microkernel requires an arm-none-eabi- toolchain with Cortex-M4
hardfloat support. The known working toolchains are as following
* [Sourcery CodeBench](http://www.mentor.com/embedded-software/sourcery-tools/sourcery-codebench/editions/lite-edition/)
  - ARM Processors: EABI Release
  - Verified versions: arm-2012.03, arm-2013.05
* [GNU Tools for ARM Embedded Processors](https://launchpad.net/gcc-arm-embedded)

For flashing and debugging on the STM32F40x, [stlink](https://github.com/texane/stlink) is required.
With `stlink` in your path, command "make flash" will flash your
STM32F4DISCOVERY board with built F9 binary image.

Other build dependency includes: (for Debian/Ubuntu)
* libncurses5-dev

When developing on top of F9 Microkernel, you do not have the luxury of using
a source level debugger such as gdb. There are still a number of techniques at
your disposal to assist debugging, however. KDB (in-kernel debugger) is built and
run at boot by default, and here are the supported commands:

* a: dump address spaces
* m: dump memory pools
* t: dump threads
* s: show softirqs
* n: show timer (now)
* e: dump ktimer events
* K: print kernel tables

Through USART, KDB can be operated interactively on PA0 (TX) and PA1 (RX) of
STM32F4DISCOVERY. You can established USART serial connection with the board
using a serial to USB converter:

* USB2TTL RX ---> PA0
* USB2TTL TX ---> PA1

Select the appropriate terminal emulator and configure it for 115200 baud,
8 data bits, no parity, one stop bit. For GNU/Linux, program `screen` can be
used for such purpose. Installation on Ubuntu / Debian based systems:

    sudo apt-get install screen

Then, attach the device file where a serial to USB converter is attached:

    screen /dev/ttyUSB0 115200 8n1

Once you want to quit screen, press: Ctrl-a k


Build Configurations
====================

F9 Microkernel deploys Linux Kernel style build system, and the corresponding
files are described as following:

* toolchain.mk:
  - toolchain-specific configurations; common cflags and ldflags
* platform/build.mk:
  - platform-specific configurations; cflags/ldflags for CPU and FPU
* board/`<BOARD_NAME>`/build.mk:
  - board-specific configurations; CHIP model, periperals
* rules.mk: the magic of build system

Configuration is the initial step in the build of F9 Microkernel for your
target, and you can use 'make config' to specify the options from which to
choose. Regardless of the configuration method you use or the actual
configuration options you choose, the build system will generate a .config
file at the end of the configuration and will generate a configuration header
file for C programs.

You can modify source file board/`<BOARD_NAME>`/board.[ch] to specify the
preferable resource assignment. To get acquainted with the configuration of
F9 Microkernel internals, file include/config.h is the entry point:

* `CONFIG_DEBUG`
  - Enable serial input/out for debugging purpose. An additional service for
    serial I/O character operations will be included.
* `CONFIG_KDB`
  - Enable in-kernel debugger.
* `CONFIG_KPROBES`
  - Enable kernel probes, the lightweight dynamic instrumentation system.
* `CONFIG_BITMAP_BITBAND`
  - Generate bitmap address in bit-band region
  - Bit-banding maps a complete word of memory onto a single bit in the
    bit-band region. For example, writing to one of the alias words will set
    or clear the corresponding bit in the bitband region.
  - When writing to the alias regions bit 0 of the 32 bit word is used to set
    the value at the bit-banding region. Reading from the alias address will
    return the value from the bit-band region in bit 0 and the other bits will
    be cleared.
* `CONFIG_MAX_`xxx series
  - limits of threads, ktimer events, async events, address spaces, flexible
    pages.
* `CONFIG_PANIC_DUMP_STACK`
  - Dump kernel stack while panic.
