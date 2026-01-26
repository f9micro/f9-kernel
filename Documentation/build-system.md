# Build System

This document describes the internal workings of the F9 microkernel build system.

## Directory Structure

The build entry point is the top-level `Makefile`. The build system performs the following tasks:
- Reads platform-related information to set include paths and platform-specific source files
- Sets the output directory
- Determines which source files to compile
- Sets toolchain compile flags
- Loads F9 microkernel configurations from `.config` (generated via `make config`)
- Includes `build.mk` files from subdirectories to determine compilation targets
- Builds and optionally flashes the image

The source tree layout:

```
$topdir/
├── board/
│   └── $(BOARD)/
├── kernel/
│   └── lib/
├── loader/
│   └── elf/
├── platform/
│   └── $(CHIP)/
└── user/
```

## Configuration Process

F9 uses [Kconfiglib](https://github.com/sysprog21/Kconfiglib), a Python-based Kconfig implementation, for build configuration. Kconfiglib provides the same functionality as the Linux kernel's Kconfig tools but with no compilation step and better portability.

### Kconfiglib Tools

| Tool | Purpose |
|------|---------|
| `menuconfig.py` | Interactive configuration menu |
| `genconfig.py` | Generate `include/autoconf.h` from `.config` |
| `defconfig.py` | Apply default configuration |
| `oldconfig.py` | Update `.config` with new options |
| `savedefconfig.py` | Save minimal configuration |

### Configuration Sequence

Running `make config` (or `make menuconfig`) triggers:

1. Kconfiglib is cloned from the repository if not present in `tools/kconfig/`
2. `menuconfig.py` runs with the root `Kconfig` file
3. User selections are saved to `.config`
4. `genconfig.py` generates `include/autoconf.h` with `CONFIG_*` macros

### Board-Specific Defaults

Each board has a default configuration in `board/$(BOARD)/defconfig`:

```bash
make discoveryf4_defconfig    # Apply STM32F4-Discovery defaults
make discoveryf429_defconfig  # Apply STM32F429I-Discovery defaults
```

## Build System Components

The `mk/` directory contains the core build logic:

```
mk/
├── config.mk
├── generic.mk
├── loader.mk
├── rules/
│   └── symmap.mk
├── target.mk
└── toolchain.mk
```

### config.mk

Legacy configuration helper. Configuration is now handled by Kconfiglib in `tools/kconfig/`. This file remains for compatibility notes.

### generic.mk

The primary build orchestrator that handles:
- Configuration generation
- Source file compilation
- Image building (when `CONFIG_LOADER` is disabled)
- Image flashing
- Build rules for different file types (`.c`, `.s`, `.bin`)
- Clean targets

### loader.mk

Active only when `CONFIG_LOADER` is enabled. Handles image building for configurations that use a separate loader stage.

### target.mk

Entry point for `make all`. Selects the appropriate linker script based on whether `CONFIG_LOADER` is set.

### toolchain.mk

Configures the cross-compilation toolchain and sets compiler flags for the target architecture.

### rules/symmap.mk

Generates symbol map code for the KDB sampling command when `CONFIG_SYMMAP` is enabled. This allows runtime symbol resolution for debugging purposes.

## Build Options

### Verbose Output

Enable detailed build output:

```bash
make V=1
```

### Platform Selection

Configure the target board in the top-level `Makefile`:
- `BOARD`: Target board identifier (e.g., `discoveryf4`, `discoveryf429`)
- `PROJECT`: Project-specific configuration

### Toolchain Configuration

Modify `mk/toolchain.mk` to adjust:

- Cross-compiler prefix
- Compiler flags
- Linker flags
- Architecture-specific options

## Output Artifacts

Build outputs are placed in `build/$(BOARD)/`:
- `f9.elf`: ELF executable with debug symbols
- `f9.bin`: Raw binary for flashing
- Object files and dependency files
