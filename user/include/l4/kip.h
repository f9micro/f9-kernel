/*********************************************************************
 *                
 * Copyright (C) 2001-2004, 2006-2007, 2010-2011,  Karlsruhe University
 *                
 * File path:     l4/kip.h
 * Description:   Kernel interface page definitions.
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
 * $Id: kip.h,v 1.36 2007/06/04 12:23:13 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __L4__KERNEL_IFACE_PAGE_H__
#define __L4__KERNEL_IFACE_PAGE_H__

#include <l4/types.h>
#include <l4/kip_types.h>
#include __L4_INC_ARCH(syscalls.h)


#if defined(L4_64BIT)
# define __PAD32	__pad32:32
# define __PLUS32	+ 32
#else
# define __PAD32	:0
# define __PLUS32
#endif


typedef union {
    L4_Word_t	raw;
    struct {
	L4_BITFIELD4( L4_Word_t,
	    __padding:16,
	    subid:8,
	    id:8,
	    __PAD32
	);
    } x;
} L4_KernelId_t;


typedef union {
    L4_Word_t	raw;
    struct {
	L4_BITFIELD4( L4_Word_t,
	    __padding:16,
	    subversion:8,
	    version:8,
	    __PAD32
	);
    } x;
} L4_ApiVersion_t;


typedef union {
    L4_Word_t	raw;
    struct {
	L4_BITFIELD3( L4_Word_t,
	    ee:2,
	    ww:2,
	    __padding:28 __PLUS32
	);
    } x;
} L4_ApiFlags_t;


typedef struct {
    L4_BITFIELD2( L4_Word_t,
	MemDescPtr:L4_BITS_PER_WORD / 2,
	n:L4_BITS_PER_WORD / 2
    );
} L4_MemoryInfo_t;



typedef struct {
    L4_Word_t		magic;
    L4_ApiVersion_t	ApiVersion;
    L4_ApiFlags_t	ApiFlags;
    L4_Word_t		KernelVerPtr;

    /* 0x10 */
    L4_Word_t		__padding10[17];
    L4_MemoryInfo_t	MemoryInfo;
    L4_Word_t		__padding58[2];

    /* 0x60 */
    struct {
	L4_Word_t	low;
	L4_Word_t	high;
    } MainMem;

    /* 0x68 */
    struct {
	L4_Word_t	low;
	L4_Word_t	high;
    } ReservedMem[2];

    /* 0x78 */
    struct {
	L4_Word_t	low;
	L4_Word_t	high;
    } DedicatedMem[5];

    /* 0xA0 */
    L4_Word_t		__paddingA0[2];

    union {
	L4_Word_t	raw;
	struct {
	    L4_BITFIELD4( L4_Word_t,
		m:10,
		a:6,
		s:6,
		__padding:10 __PLUS32
	    );
	} X;
    } UtcbAreaInfo;

    union {
	L4_Word_t	raw;
	struct {
	    L4_BITFIELD2( L4_Word_t,
		s:6,
		__padding:26 __PLUS32
	    );
	} X;
    } KipAreaInfo;

    /* 0xB0 */
    L4_Word_t		__paddingB0[2];
    L4_Word_t		BootInfo;
    L4_Word_t		ProcDescPtr;

    /* 0xC0 */
    union {
	L4_Word_t	raw;
	struct {
	    L4_BITFIELD3( L4_Word_t,
		ReadPrecision:16,
		SchedulePrecision:16,
		__PAD32
	    );
	} X;
    } ClockInfo;

    union {
	L4_Word_t	raw;
	struct {
	    L4_BITFIELD4( L4_Word_t,
		t:8,
		SystemBase:12,
		UserBase:12,
		__PAD32
	    );
	} X;
    } ThreadInfo;

    union {
	L4_Word_t	raw;
	struct {
	    L4_BITFIELD3( L4_Word_t,
		rwx:3,
		__padding:7,
		page_size_mask:22 __PLUS32
	    );
	} X;
    } PageInfo;		

    union {
	L4_Word_t	raw;
	struct {
	    L4_BITFIELD3( L4_Word_t,
		processors:16,
		__pading:12 __PLUS32,
		s:4
	    );
	} X;
    } ProcessorInfo;

    /* 0xD0 */
    L4_Word_t	SpaceControl;
    L4_Word_t	ThreadControl;
    L4_Word_t	ProcessorControl;
    L4_Word_t	MemoryControl;

    /* 0xE0 */
    L4_Word_t	Ipc;
    L4_Word_t	Lipc;
    L4_Word_t	Unmap;
    L4_Word_t	ExchangeRegisters;

    /* 0xF0 */
    L4_Word_t	SystemClock;
    L4_Word_t	ThreadSwitch;
    L4_Word_t	Schedule;
    L4_Word_t	__paddingF0;

    /* 0x100 */
    L4_Word_t	__padding100[4];

    /* 0x110 */
    L4_Word_t	ArchSyscall0;
    L4_Word_t	ArchSyscall1;
    L4_Word_t	ArchSyscall2;
    L4_Word_t	ArchSyscall3;
} L4_KernelInterfacePage_t;


typedef union {
    L4_Word_t	raw[4];
    struct {
	L4_Word_t	ExternalFreq;
	L4_Word_t	InternalFreq;
	L4_Word_t	__padding[2];
    } X;
} L4_ProcDesc_t;


typedef struct {
    L4_KernelId_t	KernelId;

    union {
	L4_Word_t	raw;
	struct {
	    L4_BITFIELD4( L4_Word_t,
		day:5,
		month:4,
		year:7,
		__padding:16 __PLUS32
	    );
	} X;
    } KernelGenDate;

    union {
	L4_Word_t	raw;
	struct {
	    L4_BITFIELD4( L4_Word_t,
		subsubver:16,
		subver:8,
		ver:8,
		__PAD32
	    );
	} X;
    } KernelVer;

    L4_Word_t	Supplier;
    char	VersionString[0];
} L4_KernelDesc_t;


/*
 * Magic that identifies the kernel interface page.
 */
#ifndef L4_MAGIC
#if defined(L4_BIG_ENDIAN)
# define L4_MAGIC	(('L' << 24) + ('4' << 16) + (230 << 8) + 'K')
#else
# define L4_MAGIC	(('K' << 24) + (230 << 16) + ('4' << 8) + 'L')
#endif
#endif /* L4_MAGIC  */

/* Values for API version field. */
#define L4_APIVERSION_2		(0x02)
#define L4_APIVERSION_4		(0x04)
#define L4_APIVERSION_X0	(0x83)
#define L4_APIVERSION_X1	(0x83)
#define L4_APIVERSION_X2	(0x84)
#define L4_APIVERSION_L4SEC	(0x85)
#define L4_APIVERSION_N1	(0x86)
#define L4_APISUBVERSION_X0	(0x80)
#define L4_APISUBVERSION_X1	(0x81)
#define L4_APIVERSION_OKL	(0xA0)

/* Values for API flags field. */
#define L4_APIFLAG_LE		(0x00)
#define L4_APIFLAG_BE		(0x01)
#define L4_APIFLAG_32BIT	(0x00)
#define L4_APIFLAG_64BIT	(0x01)

/* Values for kernel id/subid field. */
#define L4_KID_L4_486		((0 << 8) + 1)
#define L4_KID_L4_PENTIUM	((0 << 8) + 2)
#define L4_KID_L4_X86		((0 << 8) + 3)
#define L4_KID_L4_MIPS		((1 << 8) + 1)
#define L4_KID_L4_ALPHA		((2 << 8) + 1)
#define L4_KID_FIASCO		((3 << 8) + 1)
#define L4_KID_L4KA_HAZELNUT	((4 << 8) + 1)
#define L4_KID_L4KA_PISTACHIO	((4 << 8) + 2)
#define L4_KID_L4KA_STRAWBERRY	((4 << 8) + 3)
#define L4_KID_NICTA_PISTACHIO	((5 << 8) + 1)

/* Values for kernel Supplier field. */
#if defined(L4_BIG_ENDIAN)
# define L4_SUPL_GMD	(('G' << 24) + ('M' << 16) + ('D' << 8) + ' ')
# define L4_SUPL_IBM	(('I' << 24) + ('B' << 16) + ('M' << 8) + ' ')
# define L4_SUPL_UNSW	(('U' << 24) + ('N' << 16) + ('S' << 8) + 'W')
# define L4_SUPL_UKA	(('U' << 24) + ('K' << 16) + ('a' << 8) + ' ')
# define L4_SUPL_NICTA	(('N' << 24) + ('I' << 16) + ('C' << 8) + 'T')
#else
# define L4_SUPL_GMD	((' ' << 24) + ('D' << 16) + ('M' << 8) + 'G')
# define L4_SUPL_IBM	((' ' << 24) + ('M' << 16) + ('B' << 8) + 'I')
# define L4_SUPL_UNSW	(('W' << 24) + ('S' << 16) + ('N' << 8) + 'U')
# define L4_SUPL_UKA	((' ' << 24) + ('a' << 16) + ('K' << 8) + 'U')
# define L4_SUPL_NICTA	(('T' << 24) + ('C' << 16) + ('I' << 8) + 'N')
#endif



/*
 * Derived functions
 */

L4_INLINE void * L4_GetKernelInterface (void)
{
    L4_ApiVersion_t version;
    L4_ApiFlags_t flags;
    L4_KernelId_t kernel_id;

    return L4_KernelInterface (&version.raw, &flags.raw, &kernel_id.raw);
}

#if defined(__cplusplus)
L4_INLINE void * L4_KernelInterface (void)
{
    return L4_GetKernelInterface ();
}
#endif

L4_INLINE L4_Word_t L4_ApiVersion (void)
{
    L4_ApiVersion_t version;
    L4_ApiFlags_t flags;
    L4_KernelId_t kernel_id;

    L4_KernelInterface (&version.raw, &flags.raw, &kernel_id.raw);

    return version.raw;
}

L4_INLINE L4_Word_t L4_ApiFlags (void)
{
    L4_ApiVersion_t version;
    L4_ApiFlags_t flags;
    L4_KernelId_t kernel_id;

    L4_KernelInterface (&version.raw, &flags.raw, &kernel_id.raw);

    return flags.raw;
}

L4_INLINE L4_Word_t L4_KernelId (void)
{
    L4_ApiVersion_t version;
    L4_ApiFlags_t flags;
    L4_KernelId_t kernel_id;

    L4_KernelInterface (&version.raw, &flags.raw, &kernel_id.raw);

    return kernel_id.raw;
}

L4_INLINE void L4_KernelGenDate (void * KernelInterface, L4_Word_t * year,
				 L4_Word_t * month, L4_Word_t * day)
{
    L4_KernelInterfacePage_t * kip;
    L4_Word_t gen;

    kip = (L4_KernelInterfacePage_t *) KernelInterface;
    gen = ((L4_Word_t*)((L4_Word_t) KernelInterface + kip->KernelVerPtr))[1];

    *year = ((gen >> 9) & 0x7f) + 2000;
    *month = ((gen >> 5) & 0x0f);
    *day = (gen & 0x1f);
}

L4_INLINE L4_Word_t L4_KernelVersion (void * KernelInterface)
{
    L4_KernelInterfacePage_t * kip;
    kip = (L4_KernelInterfacePage_t *) KernelInterface;

    return ((L4_Word_t*)((L4_Word_t) KernelInterface + kip->KernelVerPtr))[2];
}

L4_INLINE L4_Word_t L4_KernelSupplier (void * KernelInterface)
{
    L4_KernelInterfacePage_t * kip;
    kip = (L4_KernelInterfacePage_t *) KernelInterface;

    return ((L4_Word_t*)((L4_Word_t) KernelInterface + kip->KernelVerPtr))[3];
}

L4_INLINE L4_Word_t L4_NumProcessors (void * KernelInterface)
{
    L4_KernelInterfacePage_t * kip;
    kip = (L4_KernelInterfacePage_t *) KernelInterface;

    return kip->ProcessorInfo.X.processors + 1;
}

L4_INLINE L4_Word_t L4_NumMemoryDescriptors (void * KernelInterface)
{
    L4_KernelInterfacePage_t * kip;
    kip = (L4_KernelInterfacePage_t *) KernelInterface;

    return kip->MemoryInfo.n;
}

L4_INLINE L4_Word_t L4_PageSizeMask (void * KernelInterface)
{
    L4_KernelInterfacePage_t * kip;
    kip = (L4_KernelInterfacePage_t *) KernelInterface;

    return kip->PageInfo.X.page_size_mask << 10;
}

L4_INLINE L4_Word_t L4_PageRights (void * KernelInterface)
{
    L4_KernelInterfacePage_t * kip;
    kip = (L4_KernelInterfacePage_t *) KernelInterface;

    return kip->PageInfo.X.rwx;
}

L4_INLINE L4_Word_t L4_ThreadIdBits (void * KernelInterface)
{
    L4_KernelInterfacePage_t * kip;
    kip = (L4_KernelInterfacePage_t *) KernelInterface;

    return kip->ThreadInfo.X.t;
}

L4_INLINE L4_Word_t L4_ThreadIdSystemBase (void * KernelInterface)
{
    L4_KernelInterfacePage_t * kip;
    kip = (L4_KernelInterfacePage_t *) KernelInterface;

    return kip->ThreadInfo.X.SystemBase;
}

L4_INLINE L4_Word_t L4_ThreadIdUserBase (void * KernelInterface)
{
    L4_KernelInterfacePage_t * kip;
    kip = (L4_KernelInterfacePage_t *) KernelInterface;

    return kip->ThreadInfo.X.UserBase;
}

L4_INLINE L4_Word_t L4_ReadPrecision (void * KernelInterface)
{
    L4_KernelInterfacePage_t * kip;
    kip = (L4_KernelInterfacePage_t *) KernelInterface;

    return kip->ClockInfo.X.ReadPrecision;
}

L4_INLINE L4_Word_t L4_SchedulePrecision (void * KernelInterface)
{
    L4_KernelInterfacePage_t * kip;
    kip = (L4_KernelInterfacePage_t *) KernelInterface;

    return kip->ClockInfo.X.SchedulePrecision;
}

L4_INLINE L4_Word_t L4_UtcbAreaSizeLog2 (void * KernelInterface)
{
    L4_KernelInterfacePage_t * kip;
    kip = (L4_KernelInterfacePage_t *) KernelInterface;

    return kip->UtcbAreaInfo.X.s;
}

L4_INLINE L4_Word_t L4_UtcbAreaSize (void * KernelInterface)
{
    return 1UL << L4_UtcbAreaSizeLog2 (KernelInterface);
}

L4_INLINE L4_Word_t L4_UtcbAlignmentLog2 (void * KernelInterface)
{
    L4_KernelInterfacePage_t * kip;
    kip = (L4_KernelInterfacePage_t *) KernelInterface;

    return kip->UtcbAreaInfo.X.a;
}

L4_INLINE L4_Word_t L4_UtcbSize (void * KernelInterface)
{
    L4_KernelInterfacePage_t * kip;
    kip = (L4_KernelInterfacePage_t *) KernelInterface;

    return (1UL << kip->UtcbAreaInfo.X.a) * kip->UtcbAreaInfo.X.m;
}

L4_INLINE L4_Word_t L4_KipAreaSizeLog2 (void * KernelInterface)
{
    L4_KernelInterfacePage_t * kip;
    kip = (L4_KernelInterfacePage_t *) KernelInterface;

    return kip->KipAreaInfo.X.s;
}

L4_INLINE L4_Word_t L4_KipAreaSize (void * KernelInterface)
{
    return 1UL << L4_KipAreaSizeLog2 (KernelInterface);
}

L4_INLINE L4_Word_t L4_BootInfo (void * KernelInterface)
{
    L4_KernelInterfacePage_t * kip;
    kip = (L4_KernelInterfacePage_t *) KernelInterface;

    return kip->BootInfo;
}

L4_INLINE char * L4_KernelVersionString (void * KernelInterface)
{
    L4_KernelInterfacePage_t * kip;
    kip = (L4_KernelInterfacePage_t *) KernelInterface;

    return (char *) ((L4_Word_t) KernelInterface + kip->KernelVerPtr +
		     sizeof (L4_Word_t) * 4);
}

L4_INLINE char * L4_Feature (void * KernelInterface, L4_Word_t num)
{
    char * str = L4_KernelVersionString (KernelInterface);

    do {
	while (*str++ != 0) {}
	if (*str == 0)
	    return (char *) 0;
    } while (num--);

    return str;
}
#if defined(__cplusplus)
L4_INLINE L4_Bool_t L4_HasFeature (const char *feature_name)
{
    void *kip = L4_GetKernelInterface();
    char *name;

    for( L4_Word_t i = 0; (name = L4_Feature(kip,i)) != '\0'; i++ )
    {
        const char *n = name;
        const char *fn = feature_name;

	while (*n == *fn++)
            if (*n++ == 0)
                return true;
	if (*(L4_Word8_t *)n - *(L4_Word8_t *)--fn)
            return true;
    }
    return false;
}
#endif

typedef union {
    L4_Word_t	raw[2];
    struct {
	L4_BITFIELD5( L4_Word_t,
	    type	:4,
	    t		:4,
	    __padding1	:1,
	    v		:1,
	    low		:L4_BITS_PER_WORD - 10
	);
	L4_BITFIELD2( L4_Word_t,
	    __padding2	:10,
	    high	:L4_BITS_PER_WORD - 10
	);
    } x;
} L4_MemoryDesc_t;

#define L4_UndefinedMemoryType			(0x0)
#define L4_ConventionalMemoryType		(0x1)
#define L4_ReservedMemoryType			(0x2)
#define L4_DedicatedMemoryType			(0x3)
#define L4_SharedMemoryType			(0x4)
#define L4_BootLoaderSpecificMemoryType		(0xe)
#define L4_ArchitectureSpecificMemoryType	(0xf)

L4_INLINE L4_MemoryDesc_t * L4_MemoryDesc (void * KernelInterface,
					   L4_Word_t num)
{
    L4_KernelInterfacePage_t * kip;
    kip = (L4_KernelInterfacePage_t *) KernelInterface;

    if (num >= kip->MemoryInfo.n)
	return (L4_MemoryDesc_t *) 0;

    return (L4_MemoryDesc_t *)
	((L4_Word_t) kip + kip->MemoryInfo.MemDescPtr) + num;
}

L4_INLINE L4_Bool_t L4_IsMemoryDescVirtual (L4_MemoryDesc_t * m)
{
    return m->x.v;
}

L4_INLINE L4_Word_t L4_MemoryDescType (L4_MemoryDesc_t * m)
{
    return (m->x.type >= 0x0E) ? (m->x.type + (m->x.t << 4)) : (m->x.type);
}

L4_INLINE L4_Word_t L4_MemoryDescLow (L4_MemoryDesc_t * m)
{
    return m->x.low << 10;
}

L4_INLINE L4_Word_t L4_MemoryDescHigh (L4_MemoryDesc_t * m)
{
    return (m->x.high << 10) + 0x3ff;
}

#if defined(__cplusplus)
L4_INLINE L4_Bool_t L4_IsVirtual (L4_MemoryDesc_t * m)
{
    return L4_IsMemoryDescVirtual (m);
}

L4_INLINE L4_Word_t L4_Type (L4_MemoryDesc_t * m)
{
    return L4_MemoryDescType (m);
}

L4_INLINE L4_Word_t L4_Low (L4_MemoryDesc_t * m)
{
    return L4_MemoryDescLow (m);
}

L4_INLINE L4_Word_t L4_High (L4_MemoryDesc_t * m)
{
    return L4_MemoryDescHigh (m);
}
#endif /* __cplusplus */

L4_INLINE L4_ProcDesc_t * L4_ProcDesc (void * KernelInterface, L4_Word_t cpu)
{
    L4_KernelInterfacePage_t * kip;
    kip = (L4_KernelInterfacePage_t *) KernelInterface;

    if (cpu > kip->ProcessorInfo.X.processors)
	return (L4_ProcDesc_t *) 0;

    return (L4_ProcDesc_t *)
	((L4_Word_t)kip + kip->ProcDescPtr + 
	 ((1 << kip->ProcessorInfo.X.s) * cpu));
}

L4_INLINE L4_Word_t L4_ProcDescExternalFreq (L4_ProcDesc_t * p)
{
    return p->X.ExternalFreq;
}

L4_INLINE L4_Word_t L4_ProcDescInternalFreq (L4_ProcDesc_t * p)
{
    return p->X.InternalFreq;
}

#if defined(__cplusplus)
L4_INLINE L4_Word_t L4_ExternalFreq (L4_ProcDesc_t * p)
{
    return L4_ProcDescExternalFreq (p);
}

L4_INLINE L4_Word_t L4_InternalFreq (L4_ProcDesc_t * p)
{
    return L4_ProcDescInternalFreq (p);
}
#endif /* __cplusplus */


#undef __PAD32
#undef __PLUS32

#endif /* !__L4__KERNEL_IFACE_PAGE_H__ */
