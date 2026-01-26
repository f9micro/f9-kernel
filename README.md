# F9 Microkernel

F9 is an [L4](https://en.wikipedia.org/wiki/L4_microkernel_family)-inspired microkernel designed for ARM Cortex-M,
targeting real-time embedded systems with hard determinism requirements.
It implements the fundamental microkernel principles—address spaces, threads, and IPC,
while adding advanced features from industrial RTOSes.

## Design Goals
- Hard Real-Time: Deterministic scheduling with preemption-threshold support
- Security: MPU-based memory protection and isolated execution
- Efficiency: Tickless scheduling, O(1) dispatcher, energy-aware operation
- Verifiability: Formal verification properties and comprehensive test coverage

## Key Features

### Advanced Scheduling
- Priority Bitmap Scheduler: O(1) thread selection, supporting 32 priority levels
- Preemption-Threshold Scheduling (PTS): ThreadX-style deterministic scheduling reducing context switches in critical sections
- Priority Inheritance Protocol: Automatic priority boosting to prevent priority inversion
- Round-Robin: Fair scheduling within same priority level
- Tickless Operation: Wake only on scheduled events or interrupts for power efficiency

### Memory Management
- MPU-Based Protection: Hardware memory isolation using ARM MPU (8 regions)
- Flexible Pages: Power-of-2 aligned memory regions mapped to MPU regions
- Address Spaces: Composed of flexible pages with Grant/Map/Flush operations
- Memory Pools: Physical address areas with specific attributes

### Inter-Process Communication
- Synchronous IPC: L4-style message passing with blocking semantics
- Short IPC: Register-only payload (MR0-MR7) for low latency
- Full IPC: UTCB-based message copying for larger payloads
- UTCBs: Always-mapped user-level thread control blocks for fast syscall argument access

### Hardware Support
- ARM Cortex-M3/M4/M4F: Optimized for Cortex-M architecture
- NVIC: Nested Vectored Interrupt Controller integration
- Bit Banding: Hardware bit manipulation support (where available)
- FPU: Lazy context switching for Cortex-M4F floating-point unit

### Development Tools
- KDB: In-kernel debugger with thread, memory, and timer inspection
- KProbes: Dynamic instrumentation without recompilation (Linux kernel-inspired)
- Profiling: Thread uptime, stack usage, memory fragmentation analysis
- Test Suite: Automated regression tests with QEMU integration

## Documentation

Comprehensive documentation is available in the [Documentation/](Documentation/) directory:

| Document | Description |
|----------|-------------|
| [quick-start.md](Documentation/quick-start.md) | Build, flash, and debug instructions |
| [scheduler.md](Documentation/scheduler.md) | Priority bitmap scheduler and PTS implementation |
| [threads.md](Documentation/threads.md) | Thread management, TCB structure, lifecycle |
| [memory.md](Documentation/memory.md) | Memory pools, flexible pages, MPU regions |
| [ipc.md](Documentation/ipc.md) | Synchronous IPC, message passing, UTCBs |
| [interrupt.md](Documentation/interrupt.md) | NVIC integration, interrupt handling |
| [ktimer.md](Documentation/ktimer.md) | Timer subsystem, tickless scheduling |
| [kprobes.md](Documentation/kprobes.md) | Dynamic instrumentation system |
| [init-hooks.md](Documentation/init-hooks.md) | Kernel initialization sequence |
| [build-system.md](Documentation/build-system.md) | Kconfig-based build system |

## Getting Started

### Quick Build
```shell
make config    # Configure build options (Kconfig)
make           # Build F9 kernel → build/<BOARD>/f9.elf
make flash     # Flash to STM32F4 board (requires stlink)
```

### QEMU Emulation (No Hardware Required)
```shell
make qemu
```

Press `Ctrl+A` and then `X` to exit QEMU. Press `?` in KDB for debug menu (requires `CONFIG_KDB`).

### Supported Hardware
- STM32F4DISCOVERY (STM32F407VG)
- STM32F429I-DISC1 (STM32F429ZI)
- NUCLEO-F429ZI (STM32F429ZI)
- Netduino Plus 2 (STM32F405RG) - QEMU emulated

For detailed instructions including toolchain setup, serial configuration, and debugging,
see [Documentation/quick-start.md](Documentation/quick-start.md).

## Configuration Options

F9 uses a Linux kernel-style build system via [Kconfiglib](https://github.com/sysprog21/Kconfiglib).
Run `make config` to configure options via menu. Key options:

| Option | Description |
|--------|-------------|
| `CONFIG_DEBUG` | Serial I/O for debugging |
| `CONFIG_KDB` | In-kernel debugger (press `?` for menu) |
| `CONFIG_KPROBES` | Dynamic instrumentation system |
| `CONFIG_SYMMAP` | Symbol map for profiling |
| `CONFIG_KTIMER_TICKLESS` | Tickless scheduling (power efficiency) |
| `CONFIG_MAX_THREADS` | Maximum number of threads |
| `CONFIG_MAX_KT_EVENTS` | Maximum kernel timer events |
| `CONFIG_PANIC_DUMP_STACK` | Dump stack on kernel panic |
| `CONFIG_QEMU` | QEMU emulation workarounds |

For build system details, see [Documentation/build-system.md](Documentation/build-system.md).

## License
F9 Microkernel is freely redistributable under the two-clause BSD License.
See the [LICENSE](LICENSE) file for details.
