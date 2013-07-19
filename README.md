F9 Microkernel
==============

This is `F9`, an experimental microkernel used to construct flexible embedded
systems inspired by famous L4 microkernel. The motivation of F9 microkernel
is to deploy modern microkernel techniques to support running real-time and
time-sharing applications (e.g., wireless communications) for ARM Cortex-M
series microprocessors with efficiency (performanace + power consumption) and
security (memory protection + isolated execution) in mind.


Characteristics of F9 Microkernel
=================================

* F9 follows the fundamental principles of microkernels in that it implements
  address spaces, thread management, and IPC only in the privileged kernel.

* Each thread has its own TCB (Thread Control Block) and addressed by its
  global id, tweaked for ARM Cortex-M. Also dispatcher is responsible for
  switching contexts. Threads with the same priority are executed in a
  round-robin fashion.

* Memory management is split into three concepts:
  - Memory pool, which represent area of physical address space with specific
    attributes.
  - Flexible page, which describes an always size aligned region of an address
    space. Unlike other L4 implementations, Flexible pages in F9 represent MPU
    (Memory Protection Unit, available on ARM Cortex-M3/M4) region instead.
  - Address space, which is made up of these flexpages, and system calls are
    provided to manage them:
    + grant: The memory page is granted to a new user and cannot be used
      anymore by its former user.
    + map: This implements shared memory – the memory page is passed to
      another task but can be used by both tasks.
    + flush: The memory page that has been mapped to other users will be
      flushed out of their address space.

* Regarding the interaction between a user thread and the microkernel, the
  concept of UTCB (user-level thread-control blocks) is being taken on. A UTCB
  is a small thread-specific region in the thread's virtual address space,
  which is always mapped. Therefore, the access to the UTCB can never raise a
  page fault, which makes it perfect for the kernel to access system-call
  arguments, in particular IPC payload copied from/to user threads.

* The kernel provides synchronous IPC (inter-process communication), for which
  short IPC carries payload in CPU registers only and full IPC copies message
  payload via the UTCBs of the communicating parties.


Licensing
=========

F9 Microkernel is freely redistributable under the two-clause BSD License.
Use of this source code is governed by a BSD-style license that can be found
in the `LICENSE` file.


Quick Start
===========

The current hardware board F9 Microkernel supports is STM32F4DISCOVERY based
on ARM Cortex-M4, but F9 should work well on any STM32F40x microcontroller.

Building F9 Microkernel requires an arm-none-eabi- toolchain with Cortex-M4
hardfloat support. The known working toolchains are as following
* [Sourcery CodeBench](http://www.mentor.com/embedded-software/sourcery-tools/sourcery-codebench/editions/lite-edition/)
  - ARM Processors: EABI Release
  - Verified versions: arm-2012.03, arm-2013.05
* [GNU Tools for ARM Embedded Processors](https://launchpad.net/gcc-arm-embedded)

For flashing and debugging on the STM32F40x, [stlink](https://github.com/texane/stlink) is recommended.
With `stlink` in your path, "make flash" will flash your STM32F4DISCOVERY
board with proper F9 Microkernel configurations.

When developing on top of F9 Microkernel, you do not have the luxury of using
a source level debugger such as gdb. There are still a number of techniques at
your disposal to assist debugging, however. KDB (in-kernel debugger) is built and
run at boot by default, and here are the commands it supports:

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


Build Configurations
====================

F9 Microkernel deploys Linux Kernel style build system, and the corresponding
files are described as following:

* Makefile.toolchain:
  - toolchain-specific configurations; common cflags and ldflags
* platform/build.mk:
  - platform-specific configurations; cflags/ldflags for CPU and FPU
* board/ * /build.mk:
  - board-specific configurations; CHIP model, periperals
* rules.mk: the magic of build system

You can modify source file board/<BOARD_NAME>/board.[ch] to specify the
preferable resource assignment. To get acquainted with the configuration of
F9 Microkernel internals, file include/config.h is the entry point:

* `CONFIG_DEBUG`
  - Enable serial input/out for debugging purpose. An additional system call
    for serial I/O character operations will be included.
* `CONFIG_KDB`
  - Enable in-kernel debugger.
* `CONFIG_BITMAP_BITBAND`
  - Generate bitmap address in bit-band region
  - Bit-banding maps a complete word of memory onto a single bit in the
    bit-band region. For example, writing to one of the alias words will set
    or clear the corresponding bit in the bitband region.
  - When writing to the alias regions bit 0 of the 32 bit word is used to set
    the value at the bit-banding region. Reading from the alias address will
    return the value from the bit-band region in bit 0 and the other bits will
    be cleared.
