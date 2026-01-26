# Init Hooks

The init hook mechanism provides a global initialization framework that allows code anywhere in the kernel to register functions called at specific initialization levels during system startup.

## Overview

Init hooks enable modular initialization where subsystems can register their setup routines without requiring centralized coordination. The kernel guarantees that hooks are called in order of their init level. Within a single init level, the execution order of hooks is not guaranteed.

This design allows platform-specific code, device drivers, and kernel subsystems to initialize themselves at the appropriate time during boot without modifying the core boot sequence.

## Init Levels

The following init levels are defined in ascending order of execution:

| Level | Constant | Purpose |
|-------|----------|---------|
| 0 | `INIT_LEVEL_EARLIEST` | Very early initialization |
| 1 | `INIT_LEVEL_PLATFORM_EARLY` | Early platform setup |
| 2 | `INIT_LEVEL_PLATFORM` | Platform initialization |
| 3 | `INIT_LEVEL_KERNEL` | Core kernel subsystems |
| 4 | `INIT_LEVEL_KERNEL_LATE` | Late kernel initialization |

## Usage

To register an init hook, use the `INIT_HOOK` macro with a function and the desired init level:

```c
#include <init_hook.h>
#include <debug.h>

void my_subsystem_init(void)
{
    dbg_printf(DL_EMERG, "Initializing my subsystem\n");
    /* Perform initialization */
}
INIT_HOOK(my_subsystem_init, INIT_LEVEL_PLATFORM)
```

### Specifying Relative Priority

To ensure a hook runs before a specific level, subtract from the level constant:

```c
/* Run just before platform initialization */
INIT_HOOK(early_setup, INIT_LEVEL_PLATFORM - 1)
```

To run after a level, add to the constant:

```c
/* Run just after kernel initialization */
INIT_HOOK(late_setup, INIT_LEVEL_KERNEL + 1)
```

## Implementation Details

Init hooks are implemented using linker sections. The `INIT_HOOK` macro places function pointers into a sorted section that the boot code iterates through during startup. The linker script ensures hooks are ordered by their level value.

The hook execution occurs in `kernel/start.c` after basic hardware initialization but before the scheduler starts. All hooks run in privileged mode with interrupts disabled.
