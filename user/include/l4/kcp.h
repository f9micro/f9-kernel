/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     l4/kcp.h
 * Description:   Kernel configuration page definitions used to pass
 *                paramters to L4 at boot time.
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
 * $Id: kcp.h,v 1.8 2003/09/24 19:06:21 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __L4__KCP_H__
#define __L4__KCP_H__

#include <l4/types.h>

/*
 * Magic that identifies the kernel configuration page.
 */
#ifndef L4_MAGIC
#if defined(L4_BIG_ENDIAN)
# define L4_MAGIC	(('L' << 24) + ('4' << 16) + (230 << 8) + 'K')
#else
# define L4_MAGIC	(('K' << 24) + (230 << 16) + ('4' << 8) + 'L')
#endif
#endif /* L4_MAGIC */

typedef struct L4_KernelRootServer {
    L4_Word_t   sp;
    L4_Word_t   ip;
    L4_Word_t   low;
    L4_Word_t   high;
} L4_KernelRootServer_t;

typedef struct L4_KernelConfigurationPage {
    L4_Word_t		magic;

    /* 0x04 */
    L4_Word_t		__padding04[3];

    L4_KernelRootServer_t Kdebug;
    L4_KernelRootServer_t sigma0;
    L4_KernelRootServer_t sigma1;
    L4_KernelRootServer_t root_server;

    /* 0x50 */
    L4_Word_t __padding50[1];

    struct {
       	L4_BITFIELD2( L4_Word_t, 
	    n : L4_BITS_PER_WORD / 2, 
	    MemDescPtr : L4_BITS_PER_WORD / 2 
	    );
    } MemoryInfo;	// Match declaration in kip.h

    L4_Word_t Kdebug_config[2];

    /* 0x60 */
    struct {
	L4_Word_t	low;
	L4_Word_t	high;
    } MainMem;

    /* 0x68 */
    L4_Word_t __padding68[4];

    /* 0x78 */
    struct {
	L4_Word_t	low;
	L4_Word_t	high;
    } DedicatedMem[5];

    /* 0xA0 */
    L4_Word_t		__paddingA0[6];

    /* 0xB8 */
    L4_Word_t		BootInfo;

    /* 0xBC */
    L4_Word_t		__paddingBC[1];
} L4_KernelConfigurationPage_t;


#endif /* !__L4__KCP_H__ */
