/*********************************************************************
 *                
 * Copyright (C) 2001, 2002, 2003, 2009,  Karlsruhe University
 *                
 * File path:     l4/schedule.h
 * Description:   Interfaces for doing scheduling
 *                
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *                
 * $Id: schedule.h,v 1.14 2003/09/24 19:06:21 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __L4__SCHEDULE_H__
#define __L4__SCHEDULE_H__

#include <l4/types.h>
#include __L4_INC_ARCH(syscalls.h)


/*
 * Derived functions
 */

L4_INLINE void L4_Yield (void)
{
    L4_ThreadSwitch (L4_nilthread);
}

L4_INLINE L4_Word_t L4_Set_Priority (L4_ThreadId_t tid, L4_Word_t prio)
{
    L4_Word_t dummy;
    prio &= 0xff;
    return L4_Schedule(tid, ~0UL, ~0UL, prio, ~0UL, &dummy);
}

L4_INLINE L4_Word_t L4_Set_Stride (L4_ThreadId_t tid, L4_Word_t stride)
{
    L4_Word_t dummy;
    return L4_Schedule(tid, ~0UL, ~0UL, (stride << 16) | 0xffff, ~0UL, &dummy);
}

L4_INLINE L4_Word_t L4_Set_ProcessorNo (L4_ThreadId_t tid, L4_Word_t cpu_no)
{
    L4_Word_t dummy;
    cpu_no &= 0xffff;
    return L4_Schedule(tid, ~0UL, cpu_no, ~0UL, ~0UL, &dummy);
}

L4_INLINE L4_Word_t L4_Set_Timeslice (L4_ThreadId_t tid, L4_Time_t timeslice, L4_Time_t totalquantum)
{
    L4_Word_t timectrl = (timeslice.raw << 16) | totalquantum.raw;
    return L4_Schedule(tid, timectrl, ~0UL, ~0UL, ~0UL, &timectrl);
}

L4_INLINE L4_Word_t L4_Timeslice (L4_ThreadId_t tid, L4_Time_t * timeslice, L4_Time_t * totalquantum)
{
    L4_Word_t res, timectrl;
    res = L4_Schedule(tid, ~0UL, ~0UL, ~0UL, ~0UL, &timectrl);
    timeslice->raw = timectrl >> 16;
    totalquantum->raw = timectrl;
    return res;
}

L4_INLINE L4_Word_t L4_Set_PreemptionDelay (L4_ThreadId_t tid, L4_Word_t sensitivePrio, L4_Word_t maxDelay)
{
    L4_Word_t dummy;
    L4_Word_t pctrl = ((sensitivePrio & 0xff) << 16) | (maxDelay & 0xffff);
    return L4_Schedule(tid, ~0UL, ~0UL, ~0UL, pctrl, &dummy);
}

L4_INLINE L4_Word_t L4_HS_Schedule (L4_ThreadId_t tid, L4_Word_t control, L4_ThreadId_t domain, L4_Word_t prio, L4_Word_t stride, L4_Word_t *old_control)
{
    L4_Word_t preemption_control = ((control & 0x3f) << 26) | (1 << 25);
    L4_Word_t time_control = domain.raw;
    L4_Word_t prio_control = (stride << 16) | (prio & 0x1ff);
    
    return L4_Schedule(tid, time_control, ~0UL, prio_control, preemption_control, old_control);
}



/*
 * Result values from schedule system call
 */

#define L4_SCHEDRESULT_ERROR		(0)
#define L4_SCHEDRESULT_DEAD		(1)
#define L4_SCHEDRESULT_INACTIVE		(2)
#define L4_SCHEDRESULT_RUNNING		(3)
#define L4_SCHEDRESULT_PENDING_SEND	(4)
#define L4_SCHEDRESULT_SENDING		(5)
#define L4_SCHEDRESULT_WAITING		(6)
#define L4_SCHEDRESULT_RECEIVING	(7)



#endif /* !__L4__SCHEDULE_H__ */
