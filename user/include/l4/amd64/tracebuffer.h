/*********************************************************************
 *
 * Copyright (C) 2002,  Karlsruhe University
 *
 * File path:     l4/amd64/tracebuffer.h
 * Description:   Functions for accessing the tracebuffer
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
 ********************************************************************/
#ifndef __L4__AMD64__TRACEBUFFER_H__
#define __L4__AMD64__TRACEBUFFER_H__

#include __L4_INC_ARCH(specials.h)

/**********************************************************************
 *                       Sample PMC energy weights
 **********************************************************************/
/* Pentium D */
#define X86_PMC_TSC_WEIGHT                 (1418)  
#define X86_PMC_UC_WEIGHT                  (1285)  
#define X86_PMC_LDM_WEIGHT                 (881)   
#define X86_PMC_MR_WEIGHT                  (649)   
#define X86_PMC_MB_WEIGHT                  (23421) 
#define X86_PMC_MLR_WEIGHT                 (4320)       
#define X86_PMC_RB_WEIGHT                  (840)   
#define X86_PMC_MQW_WEIGHT                 (75)    

#define X86_PMC_TSC_SHIFT                  6
#define X86_PMC_UC                         (0) 
#define X86_PMC_MLR                        (1) 
#define X86_PMC_MQW                        (4) 
#define X86_PMC_RB                         (5) 
#define X86_PMC_MB                         (12) 
#define X86_PMC_MR                         (13)          
#define X86_PMC_LDM                        (14)

L4_INLINE void L4_Tbuf_StoreRecordArch(L4_TraceRecord_t *rec, L4_TraceConfig_t config)
{
    rec->tsc = __L4_Rdtsc();
    
    if (config.X.pmon)
    {
        switch (config.X.pmon_cpu)
        {
        case 0:
            // P2/P3/K8
            rec->pmc0 = __L4_Rdpmc(0);
            rec->pmc1 = __L4_Rdpmc(1);
            break;
        case 1:
            // P4
            if (config.X.pmon_e)
            {
                L4_Word64_t pmce =
                    X86_PMC_TSC_WEIGHT *  __L4_Rdtsc() +
                    X86_PMC_UC_WEIGHT  *  __L4_Rdpmc(X86_PMC_UC)  +
                    X86_PMC_MLR_WEIGHT *  __L4_Rdpmc(X86_PMC_MLR) +
                    X86_PMC_MQW_WEIGHT *  __L4_Rdpmc(X86_PMC_MQW) +
                    X86_PMC_RB_WEIGHT  *  __L4_Rdpmc(X86_PMC_RB)  +
                    X86_PMC_MB_WEIGHT  *  __L4_Rdpmc(X86_PMC_MB)  +
                    X86_PMC_MR_WEIGHT  *  __L4_Rdpmc(X86_PMC_MR)  +
                    X86_PMC_LDM_WEIGHT *  __L4_Rdpmc(X86_PMC_LDM);
                
                rec->pmc0 = (L4_Word_t) pmce;
                rec->pmc1 = (L4_Word_t) (pmce >> 32);
            }
            else
            {
                rec->pmc0 = (L4_Word_t) __L4_Rdpmc(12);
                rec->pmc1 = (L4_Word_t) __L4_Rdpmc(14);
            }
            break;
        default:
            break;
        }
    }
}

#endif /* !__L4__AMD64__TRACEBUFFER_H__ */
