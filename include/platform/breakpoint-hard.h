#ifndef PLATFORM_HARD_BREAKPOINT_H_
#define PLATFORM_HARD_BREAKPOINT_H_

void hard_breakpoint_pool_init(void);
struct breakpoint *hard_breakpoint_config(uint32_t addr, struct breakpoint *b);
#endif
