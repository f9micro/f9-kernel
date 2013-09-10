/*********************************************************************
 *                
 * Copyright (C) 1999-2010,  Karlsruhe University
 * Copyright (C) 2008-2009,  Volkmar Uhlig, IBM Corporation
 *                
 * File path:     l4/tracebuffer.h
 * Description:   
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
 * $Id$
 *                
 ********************************************************************/

#ifndef __L4__TRACEBUFFER_H__
#define __L4__TRACEBUFFER_H__

#include <l4/thread.h>

#if defined(L4_64BIT)
# define __PLUS32       + 32
# define L4_TRACEBUFFER_MAGIC            (0x1464b123acebf)
#else
# define __PLUS32
# define L4_TRACEBUFFER_MAGIC            (0x1464b123)
#endif

#define L4_TRACEBUFFER_NUM_ARGS         (9)
#define L4_TRACEBUFFER_USERID_START	(100)
#define L4_TRACEBUFFER_DEFAULT_TYPE	(0x1)

typedef union{
    struct {
        L4_Word_t              smp      : 1; // SMP 
        L4_Word_t              pmon     : 1; // Enable perf monitoring
        L4_Word_t              pmon_cpu : 2; // CPU: x86: 00=P2/P3/K8, 01=P4
        L4_Word_t              pmon_e   : 2; // Enable energy monitoring
        L4_Word_t                       : 26 __PLUS32;
    } X;
    L4_Word_t raw;
} L4_TraceConfig_t;
    

/*
 * A tracebuffer record indicates the type of event, the time of the
 * event, the current thread, a number of event specific parameters,
 * and potentially the current performance counters.
 */
typedef struct
{
    struct {
        L4_Word_t       utype   : 16;
        L4_Word_t       __pad0  : 16 __PLUS32;
        L4_Word_t       cpu     : 16;
        L4_Word_t       id      : 16 __PLUS32;
    } X;
    
    L4_Word_t   tsc;
    L4_Word_t   thread;
    L4_Word_t   pmc0;
    L4_Word_t   pmc1;
    L4_Word_t   str;
    L4_Word_t   data[9];
} L4_TraceRecord_t;

typedef struct 
{
    L4_Word_t        magic;
    L4_Word_t        current;
    L4_Word_t        mask;
    L4_Word_t        max;
    L4_TraceConfig_t  config;
    L4_Word_t        __pad[3];
    L4_Word_t        counters[8];
    L4_TraceRecord_t tracerecords[];
} L4_TraceBuffer_t;

#include __L4_INC_ARCH(tracebuffer.h)

extern L4_TraceBuffer_t *L4_GetTraceBuffer();

L4_INLINE void L4_Tbuf_StoreRecord(L4_TraceRecord_t *rec, const L4_TraceConfig_t config, L4_Word_t type, L4_Word_t id)
{
    /* Store type, cpu, id, thread, counters */
    rec->X.utype = type;
    rec->X.id = id;
    rec->X.cpu = L4_ProcessorNo();
    rec->thread = L4_Myself().raw;
    L4_Tbuf_StoreRecordArch(rec, config);
}

L4_INLINE L4_TraceRecord_t *L4_Tbuf_NextRecord(L4_Word_t type, L4_Word_t id)
{   
    L4_TraceBuffer_t *__L4_tbuf = L4_GetTraceBuffer();   
    if (!__L4_tbuf) return 0;
            
    /* Check wheter to filter the event */                  
    if ((__L4_tbuf->mask & ((type & 0xffff) << 16)) == 0)         
        return 0;                                       
          
    //__L4_Inc_Atomic (&__L4_tbuf->current);
    if (__L4_tbuf->current == __L4_tbuf->max)
        __L4_tbuf->current = 0;

    /* Store type, cpu, id, thread, counters */
    L4_Tbuf_StoreRecord(__L4_tbuf->tracerecords + __L4_tbuf->current, __L4_tbuf->config, type, id);
     
    return __L4_tbuf->tracerecords + __L4_tbuf->current;
}

L4_INLINE void L4_Tbuf_IncCounter (L4_Word_t counter)
{
    L4_TraceBuffer_t *__L4_tbuf = L4_GetTraceBuffer();   
    if (!__L4_tbuf) return;
    __L4_tbuf->counters[counter & 0x7]++;
}

L4_INLINE void L4_Tbuf_RecordEvent_t0 (L4_Word_t id, const char * str)
{ 
    L4_TraceRecord_t *rec = L4_Tbuf_NextRecord(L4_TRACEBUFFER_DEFAULT_TYPE, id);
    if (rec == 0)
	return;
    
    rec->str = (L4_Word_t) str;
}

L4_INLINE void L4_Tbuf_RecordEvent_t1 (L4_Word_t id, const char * str,
				       L4_Word_t p0)
{
    L4_TraceRecord_t *rec = L4_Tbuf_NextRecord(L4_TRACEBUFFER_DEFAULT_TYPE, id);
    if (rec == 0)
	return;
    rec->str = (L4_Word_t) str;
    rec->data[0] = p0;
}

L4_INLINE void L4_Tbuf_RecordEvent_t2 (L4_Word_t id, const char * str,
				       L4_Word_t p0, L4_Word_t p1)
{
    L4_TraceRecord_t *rec = L4_Tbuf_NextRecord(L4_TRACEBUFFER_DEFAULT_TYPE, id);
    if (rec == 0)
	return;
    rec->str = (L4_Word_t) str;
    rec->data[0] = p0;
    rec->data[1] = p1;

}

L4_INLINE void L4_Tbuf_RecordEvent_t3 (L4_Word_t id, const char * str,
				       L4_Word_t p0, L4_Word_t p1,
				       L4_Word_t p2)
{
    L4_TraceRecord_t *rec = L4_Tbuf_NextRecord(L4_TRACEBUFFER_DEFAULT_TYPE, id);
    if (rec == 0)
	return;
    rec->str = (L4_Word_t) str;
    rec->data[0] = p0;
    rec->data[1] = p1;
    rec->data[2] = p2;
}

L4_INLINE void L4_Tbuf_RecordEvent_t4 (L4_Word_t id, const char * str,
				       L4_Word_t p0, L4_Word_t p1,
				       L4_Word_t p2, L4_Word_t p3)
{
    L4_TraceRecord_t *rec = L4_Tbuf_NextRecord(L4_TRACEBUFFER_DEFAULT_TYPE, id);
    if (rec == 0)
	return;
    rec->str = (L4_Word_t) str;
    rec->data[0] = p0;
    rec->data[1] = p1;
    rec->data[2] = p2;
    rec->data[3] = p3;
}

L4_INLINE void L4_Tbuf_RecordEvent_0 (L4_Word_t id, const char * str)
{
    L4_Tbuf_RecordEvent_t0 (id, str);
}

L4_INLINE void L4_Tbuf_RecordEvent_1 (L4_Word_t id, const char * str,
  L4_Word_t p0)
{
    L4_Tbuf_RecordEvent_t1 (id, str, p0);
}

L4_INLINE void L4_Tbuf_RecordEvent_2 (L4_Word_t id, const char * str,
				      L4_Word_t p0, L4_Word_t p1)
{
    L4_Tbuf_RecordEvent_t2 (id, str, p0, p1);
}

L4_INLINE void L4_Tbuf_RecordEvent_3 (L4_Word_t id, const char * str,
				      L4_Word_t p0, L4_Word_t p1,
				      L4_Word_t p2)
{
    L4_Tbuf_RecordEvent_t3 (id, str, p0, p1, p2);
}

L4_INLINE void L4_Tbuf_RecordEvent_4 (L4_Word_t id, const char * str,
				      L4_Word_t p0, L4_Word_t p1,
				      L4_Word_t p2, L4_Word_t p3)
{
    L4_Tbuf_RecordEvent_t4 (id, str, p0, p1, p2, p3);
}

#if defined(__cplusplus)
L4_INLINE void L4_Tbuf_RecordEvent (L4_Word_t id, const char * str)
{
    L4_Tbuf_RecordEvent_0 (id, str);
}

L4_INLINE void L4_Tbuf_RecordEvent (L4_Word_t id, const char * str,
				    L4_Word_t p0)
{
    L4_Tbuf_RecordEvent_1 (id, str, p0);
}

L4_INLINE void L4_Tbuf_RecordEvent (L4_Word_t id, const char * str,
				    L4_Word_t p0, L4_Word_t p1)
{
    L4_Tbuf_RecordEvent_2 (id, str, p0, p1);
}

L4_INLINE void L4_Tbuf_RecordEvent (L4_Word_t id, const char * str,
				    L4_Word_t p0, L4_Word_t p1,
				    L4_Word_t p2)
{
    L4_Tbuf_RecordEvent_3 (id, str, p0, p1, p2);
}

L4_INLINE void L4_Tbuf_RecordEvent (L4_Word_t id, const char * str,
				    L4_Word_t p0, L4_Word_t p1,
				    L4_Word_t p2, L4_Word_t p3)
{
    L4_Tbuf_RecordEvent_4 (id, str, p0, p1, p2, p3);
}


#endif /* __cplusplus */

#endif /* !__L4__TRACEBUFFER_H__ */
