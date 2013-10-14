#ifndef PLATFORM_SOFT_BREAKPOINT_H_
#define PLATFORM_SOFT_BREAKPOINT_H_

void soft_breakpoint_pool_init(void);
struct breakpoint *soft_breakpoint_config(uint32_t addr, struct breakpoint *b);
#endif
