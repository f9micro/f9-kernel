/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef PLATFORM_IRQ_H_
#define PLATFORM_IRQ_H_

#include <error.h>
#include <platform/cortex_m.h>
#include <platform/link.h>
#include <sched.h>
#include <softirq.h>
#include <thread.h>

void irq_init(void);

/*
 * Interrupt Priority Levels (ARM Cortex-M 4-bit priorities)
 */
#define IRQ_PRIO_ZERO_LATENCY_MAX 0x2 /* Highest priority, never masked */
#define IRQ_PRIO_SYSTICK 0x3          /* System timer */
#define IRQ_PRIO_KERNEL_MASK 0x40     /* BASEPRI mask (0x4 << 4) */
#define IRQ_PRIO_USER_DEFAULT 0x8     /* Default user IRQ priority */
#define IRQ_PRIO_LOWEST 0xF           /* SVCall, PendSV */

/*
 * System state tracking for ISR context.
 * 0 = Thread mode (PSP), 1+ = Handler mode (MSP, tracks nesting depth).
 */
extern volatile uint32_t irq_system_state;

/*
 * Fast ISR context check using hardware IPSR register.
 * Returns: true if currently in exception handler, false if in thread mode.
 * Zero overhead: Single MRS instruction, no memory access or race conditions.
 */
static inline bool in_isr_context(void)
{
    return IPSR() != 0;
}

/*
 * PRIMASK-based critical sections (blocks ALL interrupts).
 */
static inline void irq_disable(void)
{
    __asm__ __volatile__("cpsid i" ::: "memory");
}

static inline void irq_enable(void)
{
    __asm__ __volatile__("cpsie i" ::: "memory");
}

/*
 * Save and restore interrupt state (PRIMASK).
 * Used for critical sections that may be nested or called from
 * contexts where IRQ state is unknown.
 */
static inline uint32_t irq_save_flags(void)
{
    uint32_t flags;
    __asm__ __volatile__(
        "mrs %0, primask\n\t"
        "cpsid i"
        : "=r"(flags)::"memory");
    return flags;
}

static inline void irq_restore_flags(uint32_t flags)
{
    __asm__ __volatile__("msr primask, %0" ::"r"(flags) : "memory");
}

/*
 * BASEPRI-based critical sections (blocks interrupts >= priority level).
 * Zero-latency ISRs at priority 0x0-0x2 can preempt kernel critical sections.
 */
static inline void irq_disable_below(uint8_t priority)
{
    uint32_t basepri = (priority << 4) & 0xFF;
    __asm__ __volatile__("msr basepri, %0" ::"r"(basepri) : "memory");
}

static inline void irq_enable_all(void)
{
    __asm__ __volatile__("msr basepri, %0" ::"r"(0) : "memory");
}

static inline uint32_t irq_save_basepri(uint8_t priority)
{
    uint32_t prev_basepri;
    uint32_t new_basepri = (priority << 4) & 0xFF;
    __asm__ __volatile__(
        "mrs %0, basepri\n\t"
        "msr basepri, %1"
        : "=r"(prev_basepri)
        : "r"(new_basepri)
        : "memory");
    return prev_basepri;
}

static inline void irq_restore_basepri(uint32_t basepri)
{
    __asm__ __volatile__("msr basepri, %0" ::"r"(basepri) : "memory");
}

/*
 * Kernel critical section (masks interrupts >= 0x4, allows 0x0-0x3).
 * Use this as the default for scheduler, IPC, and memory operations.
 */
static inline uint32_t irq_kernel_critical_enter(void)
{
    return irq_save_basepri(IRQ_PRIO_KERNEL_MASK >> 4);
}

static inline void irq_kernel_critical_exit(uint32_t basepri)
{
    irq_restore_basepri(basepri);
}

static inline void irq_svc(void)
{
    __asm__ __volatile__("svc #0");
}

static inline int irq_number(void)
{
    int irqno;

    __asm__ __volatile__("mrs %0, ipsr" : "=r"(irqno));

    return irqno;
}

/*
 * Global to hold saved r4-r11 across irq_save_regs_only / irq_save_rest.
 * Used to capture registers before compiler can corrupt them.
 */
extern volatile uint32_t __irq_saved_regs[8];

/*
 * irq_save_regs_only()
 *
 * Saves {r4-r11} to global __irq_saved_regs immediately.
 * MUST be called at very start of naked handler before any C expressions.
 * Uses only r0-r3 which are caller-saved and safe to clobber.
 */
#define irq_save_regs_only()            \
    __asm__ __volatile__(               \
        "ldr r0, =__irq_saved_regs\n\t" \
        "stm r0, {r4-r11}" ::           \
            : "r0", "memory")

/*
 * irq_save()
 *
 * Saves {r4-r11}, msp, psp.
 * Assumes irq_save_regs_only() was called first in naked handler.
 * Copies saved regs from global to ctx->regs.
 */
#define __irq_save(ctx)                                               \
    {                                                                 \
        uint32_t *_regs = (uint32_t *) (ctx)->regs;                   \
        extern volatile uint32_t __irq_saved_regs[8];                 \
        /* Explicit unrolled copy to prevent loop mis-optimization */ \
        _regs[0] = __irq_saved_regs[0];                               \
        _regs[1] = __irq_saved_regs[1];                               \
        _regs[2] = __irq_saved_regs[2];                               \
        _regs[3] = __irq_saved_regs[3];                               \
        _regs[4] = __irq_saved_regs[4];                               \
        _regs[5] = __irq_saved_regs[5];                               \
        _regs[6] = __irq_saved_regs[6];                               \
        _regs[7] = __irq_saved_regs[7];                               \
    }                                                                 \
    __asm__ __volatile__("and r4, lr, 0xf" ::: "r4");                 \
    __asm__ __volatile__("teq r4, #0x9");                             \
    __asm__ __volatile__("ite eq");                                   \
    __asm__ __volatile__("mrseq r0, msp" ::: "r0");                   \
    __asm__ __volatile__("mrsne r0, psp" ::: "r0");                   \
    __asm__ __volatile__("mov %0, r0" : "=r"((ctx)->sp));             \
    __asm__ __volatile__("mov %0, lr" : "=r"((ctx)->ret));

#ifdef CONFIG_FPU
#define irq_save(ctx)                                                  \
    __asm__ __volatile__("cpsid i");                                   \
    (ctx)->fp_flag = 0;                                                \
    __irq_save(ctx);                                                   \
    __asm__ __volatile__("tst lr, 0x10");                              \
    __asm__ __volatile__("bne no_fp");                                 \
    __asm__ __volatile__("mov r3, %0" : : "r"((ctx)->fp_regs) : "r3"); \
    __asm__ __volatile__("vstm r3!, {d8-d15}" ::: "r3");               \
    __asm__ __volatile__("mov r4, 0x1" : : : "r4");                    \
    __asm__ __volatile__("stm r3, {r4}");                              \
    __asm__ __volatile__("no_fp:");
#else /* ! CONFIG_FPU */
#define irq_save(ctx)                \
    __asm__ __volatile__("cpsid i"); \
    __irq_save(ctx);
#endif

#define __irq_restore(ctx)                                   \
    __asm__ __volatile__("mov lr, %0" : : "r"((ctx)->ret));  \
    __asm__ __volatile__("mov r0, %0" : : "r"((ctx)->sp));   \
    __asm__ __volatile__("mov r2, %0" : : "r"((ctx)->ctl));  \
    __asm__ __volatile__("and r4, lr, 0xf" ::: "r4");        \
    __asm__ __volatile__("teq r4, #0x9");                    \
    __asm__ __volatile__("ite eq");                          \
    __asm__ __volatile__("msreq msp, r0");                   \
    __asm__ __volatile__("msrne psp, r0");                   \
    __asm__ __volatile__("mov r0, %0" : : "r"((ctx)->regs)); \
    __asm__ __volatile__("ldm r0, {r4-r11}");                \
    __asm__ __volatile__("msr control, r2\n\tisb" ::: "memory");

#ifdef CONFIG_FPU
#define irq_restore(ctx)                                                   \
    __irq_restore(ctx);                                                    \
    if ((ctx)->fp_flag) {                                                  \
        __asm__ __volatile__("mov r0, %0" : : "r"((ctx)->fp_regs) : "r0"); \
        __asm__ __volatile__("vldm r0, {d8-d15}");                         \
    }                                                                      \
    __asm__ __volatile__("cpsie i");
#else /* ! CONFIG_FPU */
#define irq_restore(ctx) \
    __irq_restore(ctx);  \
    __asm__ __volatile__("cpsie i");
#endif

/* Initial context switches to kernel.
 * It simulates interrupt to save corect context on stack.
 * When interrupts are enabled, it will jump to interrupt handler
 * and then return to normal execution of kernel code.
 */
#define init_ctx_switch(ctx, pc)                           \
    __asm__ __volatile__("mov r0, %0" : : "r"((ctx)->sp)); \
    __asm__ __volatile__("msr msp, r0");                   \
    __asm__ __volatile__("mov r1, %0" : : "r"(pc));        \
    __asm__ __volatile__("cpsie i");                       \
    __asm__ __volatile__("bx r1");

#define irq_enter() __asm__ __volatile__("push {lr}");

#define irq_return()                  \
    __asm__ __volatile__("pop {lr}"); \
    __asm__ __volatile__("bx lr");

#define context_switch(from, to)          \
    {                                     \
        __asm__ __volatile__("pop {lr}"); \
        irq_save(&(from)->ctx);           \
        thread_switch((to));              \
        irq_restore(&(to)->ctx);          \
        __asm__ __volatile__("bx lr");    \
    }

#define schedule_in_irq()                                                   \
    {                                                                       \
        register tcb_t *sel;                                                \
        sel = schedule_select();                                            \
        /* Check current thread canary before any return path.              \
         * Catches overflow that occurred while thread ran. */              \
        if (!thread_check_canary((tcb_t *) current)) {                      \
            panic(                                                          \
                "Stack overflow (current): tid=%t, "                        \
                "stack_base=%p, canary=%p\n",                               \
                current->t_globalid, current->stack_base,                   \
                current->stack_base ? *((uint32_t *) current->stack_base)   \
                                    : 0);                                   \
        }                                                                   \
        if (sel != current) {                                               \
            /* Check next thread before switching to it */                  \
            if (!thread_check_canary(sel)) {                                \
                panic(                                                      \
                    "Stack overflow (next): tid=%t, "                       \
                    "stack_base=%p, canary=%p\n",                           \
                    sel->t_globalid, sel->stack_base,                       \
                    sel->stack_base ? *((uint32_t *) sel->stack_base) : 0); \
            }                                                               \
            context_switch(current, sel);                                   \
        } else {                                                            \
            /* No context switch - restore saved registers                  \
             * and return via irq_return path */                            \
            extern volatile uint32_t __irq_saved_regs[8];                   \
            __asm__ __volatile__(                                           \
                "mov r0, %0\n\t"                                            \
                "ldm r0, {r4-r11}"                                          \
                :                                                           \
                : "r"(__irq_saved_regs)                                     \
                : "r0");                                                    \
        }                                                                   \
    }

#define request_schedule()               \
    do {                                 \
        *SCB_ICSR |= SCB_ICSR_PENDSVSET; \
    } while (0)


#define NO_PREEMPTED_IRQ (*SCB_ICSR & SCB_ICSR_RETTOBASE)

/*
 * Context is switched when the selected thead is different
 * to current then do context switch.
 *
 * Idea is that on interrupt we'll save all registers under
 * irq_stack_pointer than on return we copy registers to
 * thread's structure or to kernel_ctx
 * */

#define IRQ_HANDLER(name, sub) \
    void name(void) __NAKED;   \
    void name(void)            \
    {                          \
        irq_enter();           \
        sub();                 \
        request_schedule();    \
        irq_return();          \
    }

/*
 * SVC_HANDLER - Specialized handler for SVC exceptions requiring R4-R11
 * preservation.
 *
 * Unlike IRQ_HANDLER, this variant saves R4-R11 to __irq_saved_regs BEFORE
 * calling the C handler, ensuring message registers are captured untouched.
 *
 * This is critical for IPC fastpath optimization where user message registers
 * (MR0-MR7 in R4-R11) must be available before any C code runs.
 *
 * IMPORTANT: Must restore R4-R11 before returning since the C handler may
 * clobber these registers.
 *
 * Usage: SVC_HANDLER(svc_handler, __svc_handler);
 */
#define SVC_HANDLER(name, sub)                                            \
    void name(void) __NAKED;                                              \
    void name(void)                                                       \
    {                                                                     \
        irq_enter();                                                      \
        irq_save_regs_only();                                             \
        sub();                                                            \
        request_schedule();                                               \
        /* Restore R4-R11 before returning */                             \
        __asm__ __volatile__(                                             \
            "ldr r0, =__irq_saved_regs\n\t"                               \
            "ldm r0, {r4-r11}" ::                                         \
                : "r0", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", \
                  "memory");                                              \
        irq_return();                                                     \
    }
extern volatile tcb_t *current;

#endif /* PLATFORM_IRQ_H_ */
