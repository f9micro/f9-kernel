/*********************************************************************
 *                
 * Copyright (C) 1999-2010,  Karlsruhe University
 * Copyright (C) 2008-2009,  Volkmar Uhlig, IBM Corporation
 *                
 * File path:     l4/powerpc/arch.h
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
#ifndef __L4__POWERPC__ARCH_H__
#define __L4__POWERPC__ARCH_H__

#include "specials.h"
#include <l4/ipc.h>


#define L4_CTRLXFER_GPREGS0		(0)
#define L4_CTRLXFER_GPREGS1		(1)
#define L4_CTRLXFER_GPREGSX		(2)
#define L4_CTRLXFER_FPU			(3)
#define L4_CTRLXFER_MMU			(4)
#define L4_CTRLXFER_EXCEPT		(5)
#define L4_CTRLXFER_IVOR		(6)
#define L4_CTRLXFER_TIMER		(7)
#define L4_CTRLXFER_CONFIG		(8)
#define L4_CTRLXFER_DEBUG		(9)
#define L4_CTRLXFER_ICACHE		(10)
#define L4_CTRLXFER_DCACHE		(11)
#define L4_CTRLXFER_SHADOW_TLB		(12)
#define L4_CTRLXFER_TLB(x)		(13 + (x))
#define L4_CTRLXFER_FAULT_MASK(id)	(1 << id)

#define L4_CTRLXFER_GPREGS_SIZE		(16)
#define L4_CTRLXFER_GPREGSX_SIZE	(5)
#define L4_CTRLXFER_FPU_SIZE		(32)
#define L4_CTRLXFER_MMU_SIZE		(4)
#define L4_CTRLXFER_EXCEPT_SIZE		(11)
#define L4_CTRLXFER_IVOR_SIZE		(16)
#define L4_CTRLXFER_TIMER_SIZE		(4)
#define L4_CTRLXFER_DECR_SIZE		(2)
#define L4_CTRLXFER_CONFIG_SIZE		(6)
#define L4_CTRLXFER_DEBUG_SIZE		(18)
#define L4_CTRLXFER_ICACHE_SIZE		(9)
#define L4_CTRLXFER_DCACHE_SIZE		(9)
#define L4_CTRLXFER_SHADOW_TLB_SIZE	(4)
#define L4_CTRLXFER_TLB_SIZE		(16)

#define L4_FAULT_PAGEFAULT		2
#define L4_FAULT_EXCEPTION		3
#define L4_FAULT_HVM(id)		(6+id)
#define L4_FAULT_HVM_PROGRAM		L4_FAULT_HVM(0)
#define L4_FAULT_HVM_TLB		L4_FAULT_HVM(1)

#define L4_CTRLXFER_FAULT_ID_MASK(id)	(1ULL << id)

#define L4_CTRLXFER_GPREGS0_Rx(x)	(x)
#define L4_CTRLXFER_GPREGS1_Rx(x)	((x) - 16)
#define L4_CTRLXFER_GPREGS_XER		(0)
#define L4_CTRLXFER_GPREGS_CR		(1)
#define L4_CTRLXFER_GPREGS_CTR		(2)
#define L4_CTRLXFER_GPREGS_LR		(3)
#define L4_CTRLXFER_GPREGS_IP		(4)

#define L4_CTRLXFER_FPU_Rx(x)		(x)

#define L4_CTRLXFER_TLB_TLB0(x)		((x) * 4 + 0)
#define L4_CTRLXFER_TLB_TLB1(x)		((x) * 4 + 1)
#define L4_CTRLXFER_TLB_TLB2(x)		((x) * 4 + 2)
#define L4_CTRLXFER_TLB_PID(x)		((x) * 4 + 3)

#define L4_CTRLXFER_EXCEPT_MSR		(0)
#define L4_CTRLXFER_EXCEPT_ESR		(1)
#define L4_CTRLXFER_EXCEPT_MCSR		(2)
#define L4_CTRLXFER_EXCEPT_DEAR		(3)
#define L4_CTRLXFER_EXCEPT_SRR0		(4)
#define L4_CTRLXFER_EXCEPT_SRR1		(5)
#define L4_CTRLXFER_EXCEPT_CSRR0	(6)
#define L4_CTRLXFER_EXCEPT_CSRR1	(7)
#define L4_CTRLXFER_EXCEPT_MCSRR0	(8)
#define L4_CTRLXFER_EXCEPT_MCSRR1	(9)
#define L4_CTRLXFER_EXCEPT_EVENT	(10)

/* Event injects */
#define L4_CTRLXFER_EXCEPT_MACHINE_CHECK	(1 << 0)
#define L4_CTRLXFER_EXCEPT_DEBUG		(1 << 1)
#define L4_CTRLXFER_EXCEPT_CRITICAL_INPUT	(1 << 2)
#define L4_CTRLXFER_EXCEPT_EXTERNAL_INPUT	(1 << 3)

#define L4_CTRLXFER_IVOR_IVORx(x)	(x)

#define L4_CTRLXFER_TIMER_TCR		(0)
#define L4_CTRLXFER_TIMER_TSR		(1)
#define L4_CTRLXFER_TIMER_TBL		(2)
#define L4_CTRLXFER_TIMER_TBU		(3)
#define L4_CTRLXFER_DECR_DEC		(4)
#define L4_CTRLXFER_DECR_DECAR		(5)

#define L4_CTRLXFER_CONFIG_CCR0		(0)
#define L4_CTRLXFER_CONFIG_CCR1		(1)
#define L4_CTRLXFER_CONFIG_RSTCFG	(2)
#define L4_CTRLXFER_CONFIG_IVPR		(3)
#define L4_CTRLXFER_CONFIG_PIR		(4)
#define L4_CTRLXFER_CONFIG_PVR		(5)

#define L4_CTRLXFER_DEBUG_DBSR		(0)
#define L4_CTRLXFER_DEBUG_DBDR		(1)
#define L4_CTRLXFER_DEBUG_DBCRx(x)	(2 + (x))
#define L4_CTRLXFER_DEBUG_DACx(x)	(5 + (x))
#define L4_CTRLXFER_DEBUG_DVCx(x)	(7 + (x))
#define L4_CTRLXFER_DEBUG_IACx(x)	(9 + (x))
#define L4_CTRLXFER_DEBUG_ICDBDR	(13)
#define L4_CTRLXFER_DEBUG_ICDBTx(x)	(14 + (x))
#define L4_CTRLXFER_DEBUG_DCDBTx(x)	(15 + (x))

#define L4_CTRLXFER_CACHE_VLIM		(0)
#define L4_CTRLXFER_CACHE_NVx(x)	(1 + (x))
#define L4_CTRLXFER_CACHE_TVx(x)	(5 + (x))

/*
 * PPC GPRegs 
 */

typedef union
{
    struct
    {
	L4_Word_t r[16];
    };
    L4_Word_t reg[];
} L4_PPC_GPRegs_t;


/*
 * PPC GPRegs CtrlXfer Item
 */

typedef union {
    L4_Word_t	raw[];
    struct {
	L4_CtrlXferItem_t  item;
	L4_PPC_GPRegs_t   regs;
    };
} L4_GPRegsCtrlXferItem_t;



L4_INLINE void L4_GPRegs0CtrlXferItemInit(L4_GPRegsCtrlXferItem_t *c)
{
    L4_Word_t i;
    L4_CtrlXferItemInit(&c->item, L4_CTRLXFER_GPREGS0);
    for (i=0; i < L4_CTRLXFER_GPREGS_SIZE; i++)
	c->regs.reg[i] = 0;
    
}

L4_INLINE void L4_GPRegs1CtrlXferItemInit(L4_GPRegsCtrlXferItem_t *c)
{
    L4_Word_t i;
    L4_CtrlXferItemInit(&c->item, L4_CTRLXFER_GPREGS1);
    for (i=0; i < L4_CTRLXFER_GPREGS_SIZE; i++)
	c->regs.reg[i] = 0;
    
}

L4_INLINE void L4_GPRegsCtrlXferItemSet(L4_GPRegsCtrlXferItem_t *c, 
					L4_Word_t reg, L4_Word_t val)
{
    c->regs.reg[reg] = val;
    c->item.X.mask |= (1ul << reg);    
}

L4_INLINE void L4_MsgAppendGPRegsCtrlXferItem (L4_Msg_t * msg, L4_GPRegsCtrlXferItem_t *c)
{
    L4_MsgAppendCtrlXferItem(msg, &c->item);
}

L4_INLINE void L4_MsgPutGPRegsCtrlXferItem (L4_Msg_t * msg, L4_Word_t t, L4_GPRegsCtrlXferItem_t *c)
{
    L4_MsgPutCtrlXferItem(msg, t, &c->item);
}

L4_INLINE L4_Word_t L4_MsgGetGPRegsCtrlXferItem (L4_Msg_t *msg, L4_Word_t mr, L4_GPRegsCtrlXferItem_t *c)
{
    return L4_MsgGetCtrlXferItem(msg, mr, &c->item);
}

/*
 * PPC GPRegsX
 */

typedef union
{
    struct
    {
	L4_Word_t xer;
	L4_Word_t cr;
	L4_Word_t ctr; 
	L4_Word_t lr;
	L4_Word_t ip;
    };
    L4_Word_t reg[];
} L4_PPC_GPRegsX_t;

typedef union {
    L4_Word_t	raw[];
    struct {
	L4_CtrlXferItem_t  item;
	L4_PPC_GPRegsX_t   regs;
    };
} L4_GPRegsXCtrlXferItem_t;


L4_INLINE void L4_GPRegsXCtrlXferItemInit(L4_GPRegsXCtrlXferItem_t *c)
{
    L4_Word_t i;
    L4_CtrlXferItemInit(&c->item, L4_CTRLXFER_GPREGSX);
    for (i=0; i < L4_CTRLXFER_GPREGSX_SIZE; i++)
	c->regs.reg[i] = 0;
    
}

L4_INLINE void L4_GPRegsXCtrlXferItemSet(L4_GPRegsXCtrlXferItem_t *c, 
					 L4_Word_t reg, L4_Word_t val)
{
    c->regs.reg[reg] = val;
    c->item.X.mask |= (1ul << reg);    
}

L4_INLINE void L4_MsgAppendGPRegsXCtrlXferItem (L4_Msg_t * msg, L4_GPRegsXCtrlXferItem_t *c)
{
    L4_MsgAppendCtrlXferItem(msg, &c->item);
}

L4_INLINE void L4_MsgPutGPRegsXCtrlXferItem (L4_Msg_t * msg, L4_Word_t t, L4_GPRegsXCtrlXferItem_t *c)
{
    L4_MsgPutCtrlXferItem(msg, t, &c->item);
}

L4_INLINE L4_Word_t L4_MsgGetGPRegsXCtrlXferItem (L4_Msg_t *msg, L4_Word_t mr, L4_GPRegsXCtrlXferItem_t *c)
{
    return L4_MsgGetCtrlXferItem(msg, mr, &c->item);
}

/*
 * PPC FPU Regs
 */

L4_INLINE void L4_MsgAppendFPRegsCtrlXferItem (L4_Msg_t * msg)
{
    L4_CtrlXferItem_t  item;
    L4_CtrlXferItemInit(&item, L4_CTRLXFER_FPU);
    msg->msg[msg->tag.X.u + msg->tag.X.t + 1] = item.raw[0];
    msg->tag.X.t += 1;
}

L4_INLINE void L4_MsgPutFPRegsCtrlXferItem (L4_Msg_t * msg, L4_Word_t t)
{
    L4_CtrlXferItem_t  item;
    L4_CtrlXferItemInit(&item, L4_CTRLXFER_FPU);
    msg->msg[msg->tag.X.u + t + 1] = item.raw[0];
}


/*
 * PPC TLB Regs
 */
typedef union
{
    struct
    {
	    L4_Word_t tlb0;
	    L4_Word_t tlb1;
	    L4_Word_t tlb2;
	    L4_Word_t pid : 8;
	    L4_Word_t	  : 24;
    };
    L4_Word_t reg[];
} L4_PPC_TLB_t;

/*
 * PPC TLB Regs CtrlXfer Item
 */
typedef union {
    L4_Word_t	raw[];
    struct {
    	L4_CtrlXferItem_t  item;
    	L4_PPC_TLB_t	   regs[4];
    };
} L4_TLBCtrlXferItem_t;

L4_INLINE void L4_TLBCtrlXferItemInit(L4_TLBCtrlXferItem_t *c, int tlb)
{
    L4_Word_t i;
    L4_CtrlXferItemInit(&c->item, L4_CTRLXFER_TLB(tlb));
    for (i=0; i < L4_CTRLXFER_TLB_SIZE; i++)
	    c->regs[i / 4].reg[i % 4] = 0;
    
}

L4_INLINE void L4_ShadowTLBCtrlXferItemInit(L4_TLBCtrlXferItem_t *c)
{
    L4_Word_t i;
    L4_CtrlXferItemInit(&c->item, L4_CTRLXFER_SHADOW_TLB);
    for (i=0; i < L4_CTRLXFER_SHADOW_TLB_SIZE; i++)
	    c->regs[i / 4].reg[i % 4] = 0;
    
}

L4_INLINE void L4_TLBCtrlXferItemSet(L4_TLBCtrlXferItem_t *c, 
				     L4_Word_t reg, L4_Word_t val)
{
    c->regs[reg / 4].reg[reg % 4] = val;
    c->item.X.mask |= (1ul << reg);    
}

L4_INLINE void L4_MsgAppendTLBCtrlXferItem (L4_Msg_t * msg, L4_TLBCtrlXferItem_t *c)
{
    L4_MsgAppendCtrlXferItem(msg, &c->item);
}

L4_INLINE void L4_MsgPutTLBCtrlXferItem (L4_Msg_t * msg, L4_Word_t t, L4_TLBCtrlXferItem_t *c)
{
    L4_MsgPutCtrlXferItem(msg, t, &c->item);
}

L4_INLINE L4_Word_t L4_MsgGetTLBCtrlXferItem (L4_Msg_t *msg, L4_Word_t mr, L4_TLBCtrlXferItem_t *c)
{
    return L4_MsgGetCtrlXferItem(msg, mr, &c->item);
}

/*
 * PPC Except Regs
 */

typedef union
{
    struct
    {
	L4_Word_t msr;
	L4_Word_t esr;
	L4_Word_t mcsr;
	L4_Word_t dear;
	L4_Word_t srr0;
	L4_Word_t srr1;
	L4_Word_t csrr0;
	L4_Word_t csrr1;
	L4_Word_t mcsrr0;
	L4_Word_t mcsrr1;
	L4_Word_t event;
    };
    L4_Word_t reg[L4_CTRLXFER_EXCEPT_SIZE];
} L4_PPC_Except_t;


/*
 * PPC Except Regs CtrlXfer Item
 */

typedef union {
    L4_Word_t	raw[0];
    struct {
	L4_CtrlXferItem_t  item;
	L4_PPC_Except_t	   regs;
    };
} L4_ExceptCtrlXferItem_t;

L4_INLINE void L4_ExceptCtrlXferItemInit(L4_ExceptCtrlXferItem_t *c)
{
    L4_Word_t i;
    L4_CtrlXferItemInit(&c->item, L4_CTRLXFER_EXCEPT);
    for (i=0; i < L4_CTRLXFER_EXCEPT_SIZE; i++)
	c->regs.reg[i] = 0;
    
}

L4_INLINE void L4_ExceptCtrlXferItemSet(L4_ExceptCtrlXferItem_t *c, 
					L4_Word_t reg, L4_Word_t val)
{
    c->regs.reg[reg] = val;
    c->item.X.mask |= (1ul << reg);    
}

L4_INLINE void L4_MsgAppendExceptCtrlXferItem (L4_Msg_t * msg, L4_ExceptCtrlXferItem_t *c)
{
    L4_MsgAppendCtrlXferItem(msg, &c->item);
}

L4_INLINE void L4_MsgPutExceptCtrlXferItem (L4_Msg_t * msg, L4_Word_t t, L4_ExceptCtrlXferItem_t *c)
{
    L4_MsgPutCtrlXferItem(msg, t, &c->item);
}

L4_INLINE L4_Word_t L4_MsgGetExceptCtrlXferItem (L4_Msg_t *msg, L4_Word_t mr, L4_ExceptCtrlXferItem_t *c)
{
    return L4_MsgGetCtrlXferItem(msg, mr, &c->item);
}

/*
 * PPC Ivor CtrlXfer Item
 */

typedef union
{
    struct
    {
	L4_Word_t ivor[16];
    };
    L4_Word_t reg[];
} L4_PPC_Ivor_t;

typedef union {
    L4_Word_t	raw[];
    struct {
	L4_CtrlXferItem_t item;
	L4_PPC_Ivor_t regs;
    };
} L4_IvorCtrlXferItem_t;

L4_INLINE void L4_IvorCtrlXferItemInit(L4_IvorCtrlXferItem_t *c)
{
    L4_Word_t i;
    L4_CtrlXferItemInit(&c->item,  L4_CTRLXFER_IVOR);
    for (i=0; i < L4_CTRLXFER_IVOR_SIZE; i++)
	c->regs.reg[i] = 0;
}

L4_INLINE void L4_IvorCtrlXferItemSet(L4_IvorCtrlXferItem_t *c, 
				      L4_Word_t reg, L4_Word_t val)
{
    c->regs.reg[reg] = val;
    c->item.X.mask |= (1ul << reg);    
}

L4_INLINE void L4_MsgAppendIvorCtrlXferItem (L4_Msg_t * msg, L4_IvorCtrlXferItem_t *c)
{
    L4_MsgAppendCtrlXferItem(msg, &c->item);
}

L4_INLINE void L4_MsgPutIvorCtrlXferItem (L4_Msg_t * msg, L4_Word_t t, L4_IvorCtrlXferItem_t *c)
{
    L4_MsgPutCtrlXferItem(msg, t, &c->item);
}

L4_INLINE L4_Word_t L4_MsgGetIvorCtrlXferItem (L4_Msg_t *msg, L4_Word_t mr, L4_IvorCtrlXferItem_t *c)
{
    return L4_MsgGetCtrlXferItem(msg, mr, &c->item);
}


/*
 * PPC Timer CtrlXfer Item
 */

typedef union
{
    struct
    {
	L4_Word_t tcr;
	L4_Word_t tsr;
	L4_Word_t tbl;
	L4_Word_t tbu;
	L4_Word_t dec;
	L4_Word_t decar;
    };
    L4_Word_t reg[];
} L4_PPC_Timer_t;

typedef union {
    L4_Word_t	raw[];
    struct {
	L4_CtrlXferItem_t item;
	L4_PPC_Ivor_t   regs;
    };
} L4_TimerCtrlXferItem_t;

L4_INLINE void L4_TimerCtrlXferItemInit(L4_TimerCtrlXferItem_t *c)
{
    L4_Word_t i;
    L4_CtrlXferItemInit(&c->item, L4_CTRLXFER_TIMER);
    for (i=0; i < L4_CTRLXFER_TIMER_SIZE; i++)
	c->regs.reg[i] = 0;
}

L4_INLINE void L4_TimerCtrlXferItemSet(L4_TimerCtrlXferItem_t *c, 
				       L4_Word_t reg, L4_Word_t val)
{
    c->regs.reg[reg] = val;
    c->item.X.mask |= (1ul << reg);    
}

L4_INLINE void L4_MsgAppendTimerCtrlXferItem (L4_Msg_t * msg, L4_TimerCtrlXferItem_t *c)
{
    L4_MsgAppendCtrlXferItem(msg, &c->item);
}

L4_INLINE L4_Word_t L4_MsgGetTimerCtrlXferItem (L4_Msg_t *msg, L4_Word_t mr, L4_TimerCtrlXferItem_t *c)
{
    return L4_MsgGetCtrlXferItem(msg, mr, &c->item);
}


/*
 * PPC Config CtrlXfer Item
 */

typedef union
{
    struct
    {
	L4_Word_t ccr0;
	L4_Word_t ccr1;
	L4_Word_t rstcfg;
	L4_Word_t ivpr;
	L4_Word_t pir;
	L4_Word_t pvr;
    };
    L4_Word_t reg[];
} L4_PPC_Config_t;

typedef union {
    L4_Word_t	raw[];
    struct {
	L4_CtrlXferItem_t item;
	L4_PPC_Config_t   regs;
    };
} L4_ConfigCtrlXferItem_t;

L4_INLINE void L4_ConfigCtrlXferItemInit(L4_ConfigCtrlXferItem_t *c)
{
    L4_Word_t i;
    L4_CtrlXferItemInit(&c->item, L4_CTRLXFER_CONFIG);
    for (i=0; i < L4_CTRLXFER_CONFIG_SIZE; i++)
	c->regs.reg[i] = 0;
}

L4_INLINE void L4_ConfigCtrlXferItemSet(L4_ConfigCtrlXferItem_t *c, 
					L4_Word_t reg, L4_Word_t val)
{
    c->regs.reg[reg] = val;
    c->item.X.mask |= (1ul << reg);    
}

L4_INLINE void L4_MsgAppendConfigCtrlXferItem (L4_Msg_t * msg, L4_ConfigCtrlXferItem_t *c)
{
    L4_MsgAppendCtrlXferItem(msg, &c->item);
}

L4_INLINE void L4_MsgPutConfigCtrlXferItem (L4_Msg_t * msg, L4_Word_t t, L4_ConfigCtrlXferItem_t *c)
{
    L4_MsgPutCtrlXferItem(msg, t, &c->item);
}

L4_INLINE L4_Word_t L4_MsgGetConfigCtrlXferItem (L4_Msg_t *msg, L4_Word_t mr, L4_ConfigCtrlXferItem_t *c)
{
    return L4_MsgGetCtrlXferItem(msg, mr, &c->item);
}



/*
 * PPC Debug CtrlXfer Item
 */

typedef union
{
    struct
    {
	L4_Word_t dbsr;
	L4_Word_t dbdr;
	L4_Word_t dbcr[3];
	L4_Word_t dac[2];
	L4_Word_t dvc[2];
	L4_Word_t iac[4];
	L4_Word_t icdbdr;
	L4_Word_t icdbt[2];
	L4_Word_t dcdbt[2];
    };
    L4_Word_t reg[];
} L4_PPC_Debug_t;

typedef union {
    L4_Word_t	raw[];
    struct {
	L4_CtrlXferItem_t item;
	L4_PPC_Debug_t   regs;
    };
} L4_DebugCtrlXferItem_t;

L4_INLINE void L4_DebugCtrlXferItemInit(L4_DebugCtrlXferItem_t *c)
{
    L4_Word_t i;
    L4_CtrlXferItemInit(&c->item, L4_CTRLXFER_DEBUG);
    for (i=0; i < L4_CTRLXFER_DEBUG_SIZE; i++)
	c->regs.reg[i] = 0;
}

L4_INLINE void L4_ConfigCtrlXferItemSet(L4_DebugCtrlXferItem_t *c, 
					L4_Word_t reg, L4_Word_t val)
{
    c->regs.reg[reg] = val;
    c->item.X.mask |= (1ul << reg);    
}

L4_INLINE void L4_MsgAppendDebugCtrlXferItem (L4_Msg_t * msg, L4_DebugCtrlXferItem_t *c)
{
    L4_MsgAppendCtrlXferItem(msg, &c->item);
}

L4_INLINE L4_Word_t L4_MsgGetDebugCtrlXferItem (L4_Msg_t *msg, L4_Word_t mr, L4_DebugCtrlXferItem_t *c)
{
    return L4_MsgGetCtrlXferItem(msg, mr, &c->item);
}


/*
 * PPC Cache CtrlXfer Item
 */

typedef union
{
    struct
    {
	L4_Word_t victim_limit;
	L4_Word_t normal_victim[4];
	L4_Word_t transient_victim[4];
    };
    L4_Word_t reg[];
} L4_PPC_Cache_t;

typedef union {
    L4_Word_t	raw[];
    struct {
	L4_CtrlXferItem_t item;
	L4_PPC_Cache_t   regs;
    };
} L4_CacheCtrlXferItem_t;

L4_INLINE void L4_ICacheCtrlXferItemInit(L4_CacheCtrlXferItem_t *c)
{
    L4_Word_t i;
    L4_CtrlXferItemInit(&c->item, L4_CTRLXFER_ICACHE);
    for (i=0; i < L4_CTRLXFER_ICACHE_SIZE; i++)
	c->regs.reg[i] = 0;
}

L4_INLINE void L4_DCacheCtrlXferItemInit(L4_CacheCtrlXferItem_t *c)
{
    L4_Word_t i;
    L4_CtrlXferItemInit(&c->item, L4_CTRLXFER_DCACHE);
    for (i=0; i < L4_CTRLXFER_DCACHE_SIZE; i++)
	c->regs.reg[i] = 0;
}

L4_INLINE void L4_CacheCtrlXferItemSet(L4_CacheCtrlXferItem_t *c, 
					L4_Word_t reg, L4_Word_t val)
{
    c->regs.reg[reg] = val;
    c->item.X.mask |= (1ul << reg);    
}

L4_INLINE void L4_MsgAppendCacheCtrlXferItem (L4_Msg_t * msg, L4_CacheCtrlXferItem_t *c)
{
    L4_MsgAppendCtrlXferItem(msg, &c->item);
}

L4_INLINE L4_Word_t L4_MsgGetCacheCtrlXferItem (L4_Msg_t *msg, L4_Word_t mr, L4_CacheCtrlXferItem_t *c)
{
    return L4_MsgGetCtrlXferItem(msg, mr, &c->item);
}

#if defined(__cplusplus)
#define DECLARE_INIT_SET_APPEND(type)					\
L4_INLINE void L4_Init (L4_##type##_t *c)				\
{									\
    L4_##type##Init(c);							\
}									\
									\
L4_INLINE void L4_Set (L4_##type##_t *c, L4_Word_t reg, L4_Word_t val)	\
{									\
    L4_##type##Set(c, reg, val);					\
}									\
									\
L4_INLINE void L4_Append (L4_Msg_t *msg, L4_##type##_t *c)		\
{									\
    L4_MsgAppend##type(msg, c);						\
}                                                                       \
                                                                        \
L4_INLINE void L4_Put (L4_Msg_t *msg, L4_Word_t t, L4_##type##_t *c)    \
{									\
    L4_MsgPut##type(msg, t, c);                                         \
}                                                                       \
                                                                        \
L4_INLINE void L4_Get (L4_Msg_t *msg, L4_Word_t mr, L4_##type##_t *c)   \
{									\
    L4_MsgGet##type(msg, mr, c);                                        \
}

DECLARE_INIT_SET_APPEND(GPRegsXCtrlXferItem)
DECLARE_INIT_SET_APPEND(ExceptCtrlXferItem)
DECLARE_INIT_SET_APPEND(ConfigCtrlXferItem)
#endif

#endif /* !__L4__POWERPC__ARCH_H__ */
