/*********************************************************************
 *                
 * Copyright (C) 2001-2004, 2007, 2010,  Karlsruhe University
 *                
 * File path:     l4/types.h
 * Description:   Commonly used L4 types
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
 * $Id: types.h,v 1.45 2006/10/26 12:09:10 reichelt Exp $
 *                
 ********************************************************************/
#ifndef __L4__TYPES_H__
#define __L4__TYPES_H__

#undef L4_32BIT
#undef L4_64BIT
#undef L4_BIG_ENDIAN
#undef L4_LITTLE_ENDIAN

/*
 * Determine which architecture dependent files to include
 */

#define __L4_INC_ARCH(file) <l4/platform/file>

#include __L4_INC_ARCH(types.h)




/*
 * All types used within <arch/syscalls.h> should be declared in this
 * file.
 */

#define L4_HAVE_SIGNED_WORD

typedef L4_Word_t		L4_Bool_t;

#define L4_INLINE		static inline
#define L4_BITS_PER_WORD	(sizeof (L4_Word_t) * 8)

#if defined(L4_64BIT)
# define __PLUS32	+ (sizeof (L4_Word_t) * 8 - 32)
# define __14		32
# define __18		32
#else
# define __PLUS32	+ 0	
# define __14		14
# define __18		18
#endif

#if defined(L4_BIG_ENDIAN)
#define L4_BITFIELD2(t,a,b)		t b; t a
#define L4_BITFIELD3(t,a,b,c)		t c; t b; t a
#define L4_BITFIELD4(t,a,b,c,d)		t d; t c; t b; t a
#define L4_BITFIELD5(t,a,b,c,d,e)	t e; t d; t c; t b; t a
#define L4_BITFIELD6(t,a,b,c,d,e,f)	t f; t e; t d; t c; t b; t a
#define L4_BITFIELD7(t,a,b,c,d,e,f,g)	t g; t f; t e; t d; t c; t b; t a
#define L4_BITFIELD8(t,a,b,c,d,e,f,g,h)	t h; t g; t f; t e; t d; t c; t b; t a
#define L4_BITFIELD9(t,a,b,c,d,e,f,g,h,i)	\
	t i; t h; t g; t f; t e; t d; t c; t b; t a
#define L4_SHUFFLE2(a,b)		b,a
#define L4_SHUFFLE3(a,b,c)		c,b,a
#define L4_SHUFFLE4(a,b,c,d)		d,c,b,a
#define L4_SHUFFLE5(a,b,c,d,e)		e,d,c,b,a
#define L4_SHUFFLE6(a,b,c,d,e,f)	f,e,d,c,b,a
#define L4_SHUFFLE7(a,b,c,d,e,f,g)	g,f,e,d,c,b,a
#else
#define L4_BITFIELD2(t,a,b)		t a; t b
#define L4_BITFIELD3(t,a,b,c)		t a; t b; t c
#define L4_BITFIELD4(t,a,b,c,d)		t a; t b; t c; t d
#define L4_BITFIELD5(t,a,b,c,d,e)	t a; t b; t c; t d; t e
#define L4_BITFIELD6(t,a,b,c,d,e,f)	t a; t b; t c; t d; t e; t f
#define L4_BITFIELD7(t,a,b,c,d,e,f,g)	t a; t b; t c; t d; t e; t f; t g
#define L4_BITFIELD8(t,a,b,c,d,e,f,g,h)	t a; t b; t c; t d; t e; t f; t g; t h
#define L4_BITFIELD9(t,a,b,c,d,e,f,g,h,i)	\
	t a; t b; t c; t d; t e; t f; t g; t h; t i
#define L4_SHUFFLE2(a,b)		a,b
#define L4_SHUFFLE3(a,b,c)		a,b,c
#define L4_SHUFFLE4(a,b,c,d)		a,b,c,d
#define L4_SHUFFLE5(a,b,c,d,e)		a,b,c,d,e
#define L4_SHUFFLE6(a,b,c,d,e,f)	a,b,c,d,e,f
#define L4_SHUFFLE7(a,b,c,d,e,f,g)	a,b,c,d,e,f,g
#endif


/*
 * Error codes
 */

#define L4_ErrOk		(0)
#define L4_ErrNoPrivilege	(1)
#define L4_ErrInvalidThread	(2)
#define L4_ErrInvalidSpace	(3)
#define L4_ErrInvalidScheduler	(4)
#define L4_ErrInvalidParam	(5)
#define L4_ErrUtcbArea		(6)
#define L4_ErrKipArea		(7)
#define L4_ErrNoMem		(8)



/*
 * Fpages
 */

typedef union {
    L4_Word_t	raw;
    struct {
	L4_BITFIELD4(L4_Word_t, 
		rwx : 3,
		extended : 1,
		s : 6,
		b : 22 __PLUS32);
    } X;
} L4_Fpage_t;

#define L4_Readable		(0x04)
#define L4_Writable		(0x02)
#define L4_eXecutable		(0x01)
#define L4_FullyAccessible	(0x07)
#define L4_ReadWriteOnly	(0x06)
#define L4_ReadeXecOnly		(0x05)
#define L4_NoAccess		(0x00)

#define L4_Nilpage		((L4_Fpage_t) { raw : 0UL })
#define L4_CompleteAddressSpace	((L4_Fpage_t) { X : { L4_SHUFFLE4(0, 0, 1, 0) }})

#include __L4_INC_ARCH(specials.h)

L4_INLINE L4_Bool_t L4_IsNilFpage (L4_Fpage_t f)
{
    return f.raw == 0;
}

L4_INLINE L4_Word_t L4_Rights (L4_Fpage_t f)
{
    return f.X.rwx;
}

L4_INLINE L4_Fpage_t L4_Set_Rights (L4_Fpage_t * f, L4_Word_t rwx)
{
    f->X.rwx = rwx;
    return *f;
}

L4_INLINE L4_Fpage_t L4_FpageAddRights (L4_Fpage_t f, L4_Word_t rwx)
{
    f.X.rwx |= rwx;
    return f;
}

L4_INLINE L4_Fpage_t L4_FpageAddRightsTo (L4_Fpage_t * f, L4_Word_t rwx)
{
    f->X.rwx |= rwx;
    return *f;
}

L4_INLINE L4_Fpage_t L4_FpageRemoveRights (L4_Fpage_t f, L4_Word_t rwx)
{
    f.X.rwx &= ~rwx;
    return f;
}

L4_INLINE L4_Fpage_t L4_FpageRemoveRightsFrom (L4_Fpage_t * f, L4_Word_t rwx)
{
    f->X.rwx &= ~rwx;
    return *f;
}

#if defined(__cplusplus)
static inline L4_Fpage_t operator + (const L4_Fpage_t & f, L4_Word_t rwx)
{
    return L4_FpageAddRights (f, rwx);
}

static inline L4_Fpage_t operator += (L4_Fpage_t & f, L4_Word_t rwx)
{
    return L4_FpageAddRightsTo (&f, rwx);
}

static inline L4_Fpage_t operator - (const L4_Fpage_t & f, L4_Word_t rwx)
{
    return L4_FpageRemoveRights (f, rwx);
}

static inline L4_Fpage_t operator -= (L4_Fpage_t & f, L4_Word_t rwx)
{
    return L4_FpageRemoveRightsFrom (&f, rwx);
}
#endif /* __cplusplus */

L4_INLINE L4_Fpage_t L4_Fpage (L4_Word_t BaseAddress, L4_Word_t FpageSize)
{
    L4_Fpage_t fp;
    L4_Word_t msb = __L4_Msb (FpageSize);
    fp.raw = BaseAddress;
    fp.X.s = (1UL << msb) < FpageSize ? msb + 1 : msb;
    fp.X.rwx = L4_NoAccess;
    return fp;
}

L4_INLINE L4_Fpage_t L4_FpageLog2 (L4_Word_t BaseAddress, int FpageSize)
{
    L4_Fpage_t fp;
    fp.raw = BaseAddress;
    fp.X.s = FpageSize;
    fp.X.rwx = L4_NoAccess;
    return fp;
}

L4_INLINE L4_Word_t L4_Address (L4_Fpage_t f)
{
    return f.raw & ~((1UL << f.X.s) - 1);
}

L4_INLINE L4_Word_t L4_Size (L4_Fpage_t f)
{
    return f.X.s == 0 ? 0 : (1UL << f.X.s);
}

L4_INLINE L4_Word_t L4_SizeLog2 (L4_Fpage_t f)
{
    return f.X.s;
}


/*
 * Thread IDs
 */

typedef union {
    L4_Word_t	raw;
    struct {
	L4_BITFIELD2( L4_Word_t,
		version : __14,
		thread_no : __18);
    } X;
} L4_GthreadId_t;

typedef union {
    L4_Word_t	raw;
    struct {
	L4_BITFIELD2(L4_Word_t,
		__zeros : 6,
		local_id : 26 __PLUS32);
    } X;
} L4_LthreadId_t;

typedef union {
    L4_Word_t		raw;
    L4_GthreadId_t	global;
    L4_LthreadId_t	local;
} L4_ThreadId_t;

#define L4_nilthread      ((L4_ThreadId_t) { raw : 0UL})
#define L4_anythread      ((L4_ThreadId_t) { raw : ~0UL})
#define L4_anylocalthread ((L4_ThreadId_t) { local : { X : {L4_SHUFFLE2(0, ((1UL<<(8*sizeof(L4_Word_t)-6))-1))}}})

L4_INLINE L4_ThreadId_t L4_GlobalId (L4_Word_t threadno, L4_Word_t version)
{
    L4_ThreadId_t t;
    t.global.X.thread_no = threadno;
    t.global.X.version = version;
    return t;
}

L4_INLINE L4_Word_t L4_Version (L4_ThreadId_t t)
{
    return t.global.X.version;
}

L4_INLINE L4_Word_t L4_ThreadNo (L4_ThreadId_t t)
{
    return t.global.X.thread_no;
}

L4_INLINE L4_Bool_t L4_IsThreadEqual (const L4_ThreadId_t l,
				      const L4_ThreadId_t r)
{
    return l.raw == r.raw;
}

L4_INLINE L4_Bool_t L4_IsThreadNotEqual (const L4_ThreadId_t l,
					 const L4_ThreadId_t r)
{
    return l.raw != r.raw;
}

#if defined(__cplusplus)
static inline L4_Bool_t operator == (const L4_ThreadId_t & l,
				     const L4_ThreadId_t & r)
{
    return l.raw == r.raw;
}

static inline L4_Bool_t operator != (const L4_ThreadId_t & l,
				     const L4_ThreadId_t & r)
{
    return l.raw != r.raw;
}
#endif /* __cplusplus */

L4_INLINE L4_Bool_t L4_IsNilThread (L4_ThreadId_t t)
{
    return t.raw == 0;
}

L4_INLINE L4_Bool_t L4_IsLocalId (L4_ThreadId_t t)
{
    return t.local.X.__zeros == 0;
}

L4_INLINE L4_Bool_t L4_IsGlobalId (L4_ThreadId_t t)
{
    return t.local.X.__zeros != 0;
}



/*
 * Clock
 */

typedef union {
    L4_Word64_t		raw;
    struct {
	L4_Word32_t	low;
	L4_Word32_t	high;
    } X;
} L4_Clock_t;

#if defined(__cplusplus)
static inline L4_Clock_t operator + (const L4_Clock_t & l, const int r)
{
    return (L4_Clock_t) { raw : l.raw + r };
}

static inline L4_Clock_t operator + (const L4_Clock_t & l, const L4_Word64_t r)
{
    return (L4_Clock_t) { raw : l.raw + r };
}

static inline L4_Clock_t operator + (const L4_Clock_t & l, const L4_Clock_t r)
{
    return (L4_Clock_t) { raw : l.raw + r.raw };
}

static inline L4_Clock_t operator - (const L4_Clock_t & l, const int r)
{
    return (L4_Clock_t) { raw : l.raw - r };
}

static inline L4_Clock_t operator - (const L4_Clock_t & l, const L4_Word64_t r)
{
    return (L4_Clock_t) { raw : l.raw - r };
}

static inline L4_Clock_t operator - (const L4_Clock_t & l, const L4_Clock_t r)
{
    return (L4_Clock_t) { raw : l.raw - r.raw };
}
#endif /* __cplusplus */

L4_INLINE L4_Clock_t L4_ClockAddUsec (const L4_Clock_t c, const L4_Word64_t r)
{
    return (L4_Clock_t) { raw : c.raw + r };
}

L4_INLINE L4_Clock_t L4_ClockSubUsec (const L4_Clock_t c, const L4_Word64_t r)
{
    return (L4_Clock_t) { raw : c.raw - r };
}

#if defined(__cplusplus)
static inline L4_Bool_t operator < (const L4_Clock_t &l, const L4_Clock_t &r)
{
    return l.raw < r.raw;
}

static inline L4_Bool_t operator > (const L4_Clock_t &l, const L4_Clock_t &r)
{
    return l.raw > r.raw;
}

static inline L4_Bool_t operator <= (const L4_Clock_t &l, const L4_Clock_t &r)
{
    return l.raw <= r.raw;
}

static inline L4_Bool_t operator >= (const L4_Clock_t &l, const L4_Clock_t &r)
{
    return l.raw >= r.raw;
}

static inline L4_Bool_t operator == (const L4_Clock_t &l, const L4_Clock_t &r)
{
    return l.raw == r.raw;
}

static inline L4_Bool_t operator != (const L4_Clock_t &l, const L4_Clock_t &r)
{
    return l.raw != r.raw;
}
#endif /* __cplusplus */


L4_INLINE L4_Bool_t L4_IsClockEarlier (const L4_Clock_t l, const L4_Clock_t r)
{
    return l.raw < r.raw;
}

L4_INLINE L4_Bool_t L4_IsClockLater (const L4_Clock_t l, const L4_Clock_t r)
{
    return l.raw > r.raw;
}

L4_INLINE L4_Bool_t L4_IsClockEqual (const L4_Clock_t l, const L4_Clock_t r)
{
    return l.raw == r.raw;
}

L4_INLINE L4_Bool_t L4_IsClockNotEqual (const L4_Clock_t l, const L4_Clock_t r)
{
    return l.raw != r.raw;
}


/*
 * Time
 */

typedef union {
    L4_Word16_t	raw;
    struct {
	L4_BITFIELD3(L4_Word_t,
		m : 10,
		e : 5,
		a : 1);
    } period;
    struct {
	L4_BITFIELD4(L4_Word_t,
		m : 10,
		c : 1,
		e : 4,
		a : 1);
    } point;
} L4_Time_t;

#define L4_Never	((L4_Time_t) { raw : 0UL })
#define L4_ZeroTime	((L4_Time_t) { period : { L4_SHUFFLE3(0, 1, 0) }})

L4_INLINE L4_Time_t L4_TimePeriod (L4_Word64_t microseconds)
{
#   define __L4_SET_TIMEPERIOD(exp, man) \
	do { time.period.m = man; time.period.e = exp; } while (0)
#   define __L4_TRY_EXPONENT(N) \
	else if (microseconds < (1UL << N)) \
	    __L4_SET_TIMEPERIOD (N - 10, microseconds >> (N - 10))

    L4_Time_t time;
    time.raw = 0;

    if (__builtin_constant_p (microseconds)) {
	if (0) {}
	__L4_TRY_EXPONENT (10); __L4_TRY_EXPONENT (11);
	__L4_TRY_EXPONENT (12); __L4_TRY_EXPONENT (13);
	__L4_TRY_EXPONENT (14); __L4_TRY_EXPONENT (15);
	__L4_TRY_EXPONENT (16); __L4_TRY_EXPONENT (17);
	__L4_TRY_EXPONENT (18); __L4_TRY_EXPONENT (19);
	__L4_TRY_EXPONENT (20); __L4_TRY_EXPONENT (21);
	__L4_TRY_EXPONENT (22); __L4_TRY_EXPONENT (23);
	__L4_TRY_EXPONENT (24); __L4_TRY_EXPONENT (25);
	__L4_TRY_EXPONENT (26); __L4_TRY_EXPONENT (27);
	__L4_TRY_EXPONENT (28); __L4_TRY_EXPONENT (29);
	__L4_TRY_EXPONENT (30); __L4_TRY_EXPONENT (31);
	else
	    return L4_Never;
    } else {
	L4_Word_t l4_exp = 0;
	L4_Word_t man = microseconds;
	while (man >= (1 << 10)) {
	    man >>= 1;
	    l4_exp++;
	}
	if (l4_exp <= 31)
	    __L4_SET_TIMEPERIOD (l4_exp, man);
	else
	    return L4_Never;
    }

    return time;
#   undef __L4_TRY_EXPONENT
#   undef __L4_SET_TIMEPERIOD
}
#if 0
L4_INLINE L4_Time_t L4_TimePoint (L4_Clock_t at)
{
#   define __L4_SET_TIMEPOINT(exp, man, carry) \
        do { time.point.m = man; \
	     time.point.e = exp; \
	     time.point.c = carry; \
	     time.point.a = 1; \
	} while (0)
#   define __L4_TRY_EXPONENT(N) \
	else if (microseconds < (1 << (10+N))) \
	    __L4_SET_TIMEPOINT (N, now.raw >> N, (now.raw >> (N+10)) & 0x1)

    L4_Clock_t now = L4_SystemClock ();
    L4_Word_t int microseconds = now.raw - at.raw;
    L4_Time_t time;

    if (__builtin_constant_p (at)) {
	if (0) {}
	__L4_TRY_EXPONENT (0);  __L4_TRY_EXPONENT (1);
	__L4_TRY_EXPONENT (2);  __L4_TRY_EXPONENT (3);
	__L4_TRY_EXPONENT (4);  __L4_TRY_EXPONENT (5);
	__L4_TRY_EXPONENT (6);  __L4_TRY_EXPONENT (7);
	__L4_TRY_EXPONENT (8);  __L4_TRY_EXPONENT (9);
	__L4_TRY_EXPONENT (10); __L4_TRY_EXPONENT (11);
	__L4_TRY_EXPONENT (12); __L4_TRY_EXPONENT (13);
	__L4_TRY_EXPONENT (14); __L4_TRY_EXPONENT (15);
	else
	    return L4_Never;
    } else {
	L4_Word_t int exp = 0;
	L4_Word_t int man = microseconds;
	while (man >= (1 << 10)) {
	    man >>= 1;
	    exp++;
	}
	if (exp <= 15)
	    __L4_SET_TIMEPOINT (exp, man, (now.raw >> (exp+10)) & 0x1);
	else
	    return L4_Never;
    }

    return time;
#   undef __L4_TRY_EXPONENT
#   undef __L4_SET_TIMEPOINT
}
#endif

#if defined(__cplusplus)
#if 0
/* XXX Operators on L4_Clock_t not implemented. */
static inline L4_Time_t operator + (const L4_Time_t &l, const L4_Word_t r)
{
}

static inline L4_Time_t operator += (const L4_Time_t &l, const L4_Word_t r)
{
}

static inline L4_Time_t operator - (const L4_Time_t &l, const L4_Word_t r)
{
}

static inline L4_Time_t operator -= (const L4_Time_t &l, const L4_Word_t r)
{
}

static inline L4_Bool_t operator > (const L4_Time_t &l, const L4_Time_t &r)
{
}

static inline L4_Bool_t operator >= (const L4_Time_t &l, const L4_Time_t &r)
{
}

static inline L4_Bool_t operator < (const L4_Time_t &l, const L4_Time_t &r)
{
}

static inline L4_Bool_t operator <= (const L4_Time_t &l, const L4_Time_t &r)
{
}

static inline L4_Bool_t operator == (const L4_Time_t &l, const L4_Time_t &r)
{
}

static inline L4_Bool_t operator != (const L4_Time_t &l, const L4_Time_t &r)
{
}
#endif
#endif



#undef __14
#undef __18
#undef __PLUS32

#endif /* !__L4__TYPES_H__ */
