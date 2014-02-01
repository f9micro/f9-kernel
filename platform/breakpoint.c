#if !defined(CONFIG_KPROBES)
#error __FILE__ " is the part of KProbes implementation."
#endif

#include <platform/irq.h>
#include <platform/hw_debug.h>
#include <platform/cortex_m.h>
#include <platform/breakpoint.h>
#include <platform/breakpoint-hard.h>
#include <platform/breakpoint-soft.h>

static struct breakpoint breakpoints[BKPT_MAX_NUM];

static struct breakpoint *breakpoint_config(int id, uint32_t addr)
{
	uint16_t t;

	t = breakpoint_type_by_addr(addr);

	if (t == BKPT_HARD) {
		return hard_breakpoint_config(addr, &breakpoints[id]);
	} else if (t == BKPT_SOFT) {
		return soft_breakpoint_config(addr, &breakpoints[id]);
	}

	return NULL;
}

static struct breakpoint *get_avail_breakpoint(uint32_t addr)
{
	int i;

	for (i = 0; i < BKPT_MAX_NUM; i++) {
		if (breakpoints[i].type == BKPT_NONE)
			return breakpoint_config(i, addr);
	}
	return NULL;
}

int get_breakpoint_id(struct breakpoint *b)
{
	uint32_t addr = (uint32_t)b;
	uint32_t head_addr = (uint32_t)breakpoints;

	if ((addr >= head_addr) &&
	    (addr <= (uint32_t)&breakpoints[BKPT_MAX_NUM]))
		return (addr - head_addr) / sizeof(struct breakpoint);

	return -1;
}

void breakpoint_pool_init(void)
{
	int i;

	for (i = 0; i < FPB_MAX_COMP; i++) {
		breakpoints[i].type = BKPT_NONE;
		breakpoints[i].addr = 0;
		breakpoints[i].raw_data = 0;
	}

	hard_breakpoint_pool_init();
	soft_breakpoint_pool_init();
}

struct breakpoint *breakpoint_install(uint32_t addr)
{
	return get_avail_breakpoint(addr);
}

void breakpoint_uninstall(struct breakpoint *b)
{
	b->release(b);
}

void enable_breakpoint(struct breakpoint *b)
{
	b->enable(b);
}

void disable_breakpoint(struct breakpoint *b)
{
	b->disable(b);
}
