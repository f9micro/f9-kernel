/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/* ARM Cortex-M syscall implementations */

#include <l4/types.h>
#include <l4/utcb.h>
#include <platform/link.h>
#include <syscall.h>
#include __L4_INC_ARCH(syscalls.h)

__USER_TEXT
void *L4_KernelInterface(L4_Word_t *ApiVersion,
                         L4_Word_t *ApiFlags,
                         L4_Word_t *KernelId)
{
    return &kip_start;
}

__USER_TEXT
L4_ThreadId_t L4_ExchangeRegisters(L4_ThreadId_t dest,
                                   L4_Word_t control,
                                   L4_Word_t sp,
                                   L4_Word_t ip,
                                   L4_Word_t flags,
                                   L4_Word_t UserDefHandle,
                                   L4_ThreadId_t pager,
                                   L4_Word_t *old_control,
                                   L4_Word_t *old_sp,
                                   L4_Word_t *old_ip,
                                   L4_Word_t *old_flags,
                                   L4_Word_t *old_UserDefhandle,
                                   L4_ThreadId_t *old_pager)
{
    L4_ThreadId_t result = {0};
    /* FIXME: unimplemented */
    return result;
}

__USER_TEXT
L4_Word_t L4_ThreadControl(L4_ThreadId_t dest,
                           L4_ThreadId_t SpaceSpecifier,
                           L4_ThreadId_t Scheduler,
                           L4_ThreadId_t Pager,
                           void *UtcbLocation)
{
    register L4_Word_t r0 __asm__("r0") = dest.raw;
    register L4_Word_t r1 __asm__("r1") = SpaceSpecifier.raw;
    register L4_Word_t r2 __asm__("r2") = Scheduler.raw;
    register L4_Word_t r3 __asm__("r3") = Pager.raw;
    register L4_Word_t r4 __asm__("r4") = (L4_Word_t) UtcbLocation;

    __asm__ __volatile__("svc %[syscall_num]\n"
                         : "+r"(r0), "+r"(r1), "+r"(r2), "+r"(r3), "+r"(r4)
                         : [syscall_num] "i"(SYS_THREAD_CONTROL)
                         : "memory", "r12");

    return r0;
}

__USER_TEXT
L4_Clock_t L4_SystemClock(void)
{
    register L4_Word_t r0 __asm__("r0");
    register L4_Word_t r1 __asm__("r1");

    __asm__ __volatile__("svc %[syscall_num]\n"
                         : "=r"(r0), "=r"(r1)
                         : [syscall_num] "i"(SYS_SYSTEM_CLOCK)
                         : "memory", "r2", "r3", "r12");

    L4_Clock_t result;
    result.raw = ((uint64_t) r1 << 32) | r0;
    return result;
}

__USER_TEXT
void L4_ThreadSwitch(L4_ThreadId_t dest) {}

__USER_TEXT
L4_Word_t L4_Schedule(L4_ThreadId_t dest,
                      L4_Word_t TimeControl,
                      L4_Word_t ProcessorControl,
                      L4_Word_t PrioControl,
                      L4_Word_t PreemptionControl,
                      L4_Word_t *old_TimeControl)
{
    register L4_Word_t r0 __asm__("r0") = dest.raw;
    register L4_Word_t r1 __asm__("r1") = TimeControl;
    register L4_Word_t r2 __asm__("r2") = ProcessorControl;
    register L4_Word_t r3 __asm__("r3") = PrioControl;
    register L4_Word_t r4 __asm__("r4") = PreemptionControl;
    register L4_Word_t r5 __asm__("r5") = (L4_Word_t) old_TimeControl;

    __asm__ __volatile__("svc %[syscall_num]\n"
                         : "+r"(r0), "+r"(r1), "+r"(r2), "+r"(r3), "+r"(r4),
                           "+r"(r5)
                         : [syscall_num] "i"(SYS_SCHEDULE)
                         : "memory", "r12");

    /* Write back old_TimeControl if pointer provided */
    if (old_TimeControl)
        *old_TimeControl = r5;

    return r0;
}

__USER_TEXT
L4_Word_t L4_TimerNotify(L4_Word_t ticks,
                         L4_Word_t notify_bits,
                         L4_Word_t periodic)
{
    register L4_Word_t r0 __asm__("r0") = ticks;
    register L4_Word_t r1 __asm__("r1") = notify_bits;
    register L4_Word_t r2 __asm__("r2") = periodic;

    __asm__ __volatile__("svc %[syscall_num]\n"
                         : "+r"(r0), "+r"(r1), "+r"(r2)
                         : [syscall_num] "i"(SYS_TIMER_NOTIFY)
                         : "memory", "r3", "r12");

    return r0;
}

__USER_TEXT
L4_MsgTag_t L4_Ipc(L4_ThreadId_t to,
                   L4_ThreadId_t FromSpecifier,
                   L4_Word_t Timeouts,
                   L4_ThreadId_t *from)
{
    L4_MsgTag_t result;
    extern void *current_utcb;
    utcb_t *utcb = (utcb_t *) current_utcb;
    L4_Word_t *mr_ptr = &utcb->mr_low[0];

    register L4_Word_t r0 __asm__("r0") = to.raw;
    register L4_Word_t r1 __asm__("r1") = FromSpecifier.raw;
    register L4_Word_t r2 __asm__("r2") = Timeouts;

    /* B8 Fix: MRs stored in UTCB->mr_low[], marshaled to R4-R11 for SVC
     *
     * The ARM ABI says R4-R11 are callee-saved, so any function call
     * between L4_LoadMR() and L4_Ipc() could corrupt them. We now store
     * MRs in UTCB memory (mr_low[]) and marshal here.
     *
     * P1 Fix: Use r12 as base register for ldmia/stmia. ARM reference:
     * "If <Rn> is in the register list, behavior is UNPREDICTABLE."
     * By explicitly using r12 (outside r4-r11), we avoid this hazard.
     * We save mr_ptr to stack because SVC may clobber r0-r3 where the
     * compiler might have placed it.
     *
     * Register usage during SVC:
     * - R0-R3: IPC parameters (to, from, timeout)
     * - R4-R11: Message registers (loaded from UTCB before SVC)
     * - R12: Scratch register for ldmia/stmia base
     *
     * NOTE: The SVC handler saves R4-R11 to __irq_saved_regs on entry
     * and restores them on exit. For blocking IPC, received MRs are
     * delivered via context switch (ctx.regs[]). For non-blocking,
     * sender gets original MRs back (no reply expected).
     */
    __asm__ __volatile__(
        /* Save mr_ptr to stack (SVC may clobber the input register) */
        "mov r12, %[mr_ptr]\n"
        "push {r4-r11}\n"
        "sub sp, sp, #8\n"
        "str r12, [sp, #4]\n"
        /* Load MR0-MR7 from UTCB into R4-R11 */
        "ldmia r12, {r4-r11}\n"
        /* SVC call with MRs in r4-r11 */
        "svc %[syscall_num]\n"
        /* Restore mr_ptr and store received MRs back to UTCB */
        "ldr r12, [sp, #4]\n"
        "stmia r12, {r4-r11}\n"
        "add sp, sp, #8\n"
        "pop {r4-r11}\n"
        : "+r"(r0)
        : "r"(r1), "r"(r2), [mr_ptr] "r"(mr_ptr), [syscall_num] "i"(SYS_IPC)
        : "r12", "memory");

    result.raw = utcb->mr_low[0]; /* MR0 = tag */

    if (from)
        from->raw = r0;

    return result;
}


__USER_TEXT
L4_MsgTag_t L4_Lipc(L4_ThreadId_t to,
                    L4_ThreadId_t FromSpecifier,
                    L4_Word_t Timeouts,
                    L4_ThreadId_t *from)
{
    return L4_Ipc(to, FromSpecifier, Timeouts, from);
}

__USER_TEXT
void L4_Unmap(L4_Word_t control) {}

__USER_TEXT
L4_Word_t L4_SpaceControl(L4_ThreadId_t SpaceSpecifier,
                          L4_Word_t control,
                          L4_Fpage_t KernelInterfacePageArea,
                          L4_Fpage_t UtcbArea,
                          L4_ThreadId_t redirector,
                          L4_Word_t *old_control)
{
    L4_Word_t result = 0;
    return result;
}

__USER_TEXT
L4_Word_t L4_ProcessorControl(L4_Word_t ProcessorNo,
                              L4_Word_t InternalFrequency,
                              L4_Word_t ExternalFrequency,
                              L4_Word_t voltage)
{
    L4_Word_t result = 0;
    return result;
}

__USER_TEXT
L4_Word_t L4_MemoryControl(L4_Word_t control, const L4_Word_t *attributes)
{
    L4_Word_t result = 0;
    return result;
}

__USER_TEXT
L4_Word_t L4_NotifyWait(L4_Word_t mask)
{
    register L4_Word_t r0 __asm__("r0") = mask;

    __asm__ __volatile__("svc %[syscall_num]\n"
                         : "+r"(r0)
                         : [syscall_num] "i"(SYS_NOTIFY_WAIT)
                         : "memory", "r1", "r2", "r3", "r12");

    return r0;
}

__USER_TEXT
L4_Word_t L4_NotifyPost(L4_ThreadId_t target, L4_Word_t bits)
{
    register L4_Word_t r0 __asm__("r0") = target.raw;
    register L4_Word_t r1 __asm__("r1") = bits;

    __asm__ __volatile__("svc %[syscall_num]\n"
                         : "+r"(r0), "+r"(r1)
                         : [syscall_num] "i"(SYS_NOTIFY_POST)
                         : "memory", "r2", "r3", "r12");

    return r0;
}

__USER_TEXT
L4_Word_t L4_NotifyClear(L4_Word_t bits)
{
    register L4_Word_t r0 __asm__("r0") = bits;

    __asm__ __volatile__("svc %[syscall_num]\n"
                         : "+r"(r0)
                         : [syscall_num] "i"(SYS_NOTIFY_CLEAR)
                         : "memory", "r1", "r2", "r3", "r12");

    return r0;
}
