/*********************************************************************
 *                
 * Copyright (C) 2003, 2007, 2010,  Karlsruhe University
 *                
 * File path:     l4test/kip.cc
 * Description:   Various KIP tests
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
 * $Id: kip.cc,v 1.7 2003/09/24 19:05:54 skoglund Exp $
 *                
 ********************************************************************/
#include <l4/kip.h>
#include <l4io.h>
#include <config.h>

/* for the current arch */
#include <arch.h>

/* generic stuff */
#include "l4test.h"
#include "menu.h"
#include "assert.h"

bool check_kipptr(L4_KernelInterfacePage_t *kip, void * ptr)
{
    if (((L4_Word_t)kip + (1 << kip->KipAreaInfo.X.s) >= (L4_Word_t)ptr) 
	&& (ptr >= kip))
	return true;
    printf("ERROR: kip-ptr (%p) not within KIP area\n", ptr);
    return false;
}


/* list of kernel ids, subids and what they are */
typedef struct kid_list_t { L4_Word_t id; L4_Word_t subid; const char *kernel; const char * supplier; };
kid_list_t kid_list[] = 
{
	{ 0, 1, "L4/486", "GMD" },
	{ 0, 2, "L4/Pentium", "IBM" },
	{ 0, 3, "L4/x86", "UKa" },
	{ 1, 1, "L4/Mips", "UNSW" },
	{ 2, 1, "L4/Alpha", "TUD, UNSW" },
	{ 3, 1, "Fiasco", "TUD" },
	{ 4, 1, "L4Ka/Hazelnut", "UKa" },
	{ 4, 2, "L4Ka/Pistachio", "UKa" },
	{ 4, 3, "L4Ka/Strawberry", "UKa" },
};
#define KID_LIST_COUNT (sizeof(kid_list)/sizeof(kid_list[0]))

/* list of API versions */
#define NO_SUBVERSION (L4_Word_t)-1ULL
typedef struct api_list_t { L4_Word_t version; L4_Word_t subversion; const char *api; };
api_list_t api_list[] = 
{
	{ L4_APIVERSION_2,  NO_SUBVERSION, "Version 2" },
	{ L4_APIVERSION_X0, L4_APISUBVERSION_X0, "Experimental Version X.0" },
	{ L4_APIVERSION_X1, L4_APISUBVERSION_X1, "Experimental Version X.1" },
	{ L4_APIVERSION_X2, NO_SUBVERSION, "Experimental Version X.2" },
	{ L4_APIVERSION_4,  NO_SUBVERSION, "Version 4" },
};
#define API_LIST_COUNT (sizeof(api_list)/sizeof(api_list[0]))

/* assume these could have 2 * 3 filled in later */
const char *endianess[] = { "little", "big", "ERROR-2", "ERROR-3" };
const char *apiwidth [] = { "32-bit", "64-bit", "ERROR-2", "ERROR-3" };

#define TAG0 0
#define TAG1 1
#define TAG2 2
#define TAG3 3

/* other module functions */
void
print_kernelid( L4_Word_t kid_w )
{
	L4_KernelId_t kid;
	L4_Word_t i;
	const char *kernel, *supplier;

	kid.raw = kid_w;

	for( i = 0; i < KID_LIST_COUNT; i++ )
		if( (kid_list[i].id == kid.x.id)
		    && (kid_list[i].subid == kid.x.subid) )
			break;

	if( i != KID_LIST_COUNT )
	{
		kernel = kid_list[i].kernel;
		supplier = kid_list[i].supplier;
	}
	else
	{
		kernel = "unknown kernel";
		supplier = "unknown supplier";
	}

	printf( "KernelID reports 0x%lx.0x%lx: %s from %s\n", 
		(long) kid.x.id, (long) kid.x.subid,
		kernel, supplier );
}

void
print_version( L4_Word_t apiv )
{
	L4_ApiVersion_t api;
	L4_Word_t i;
	const char *version;

	api.raw = apiv;

	for( i = 0; i < API_LIST_COUNT; i++ )
		if( (api.x.version == api_list[i].version)
		    && ((api.x.subversion == api_list[i].subversion)
			|| (api_list[i].subversion == NO_SUBVERSION)) )
			break;

	if( i != API_LIST_COUNT )
		version = api_list[i].api;
	else
		version = "unknown api";

	printf( "APIVersion reports %d.%d: %s\n", 
		(int) api.x.version, (int) api.x.subversion,
		version );
}

void
print_apiflags( L4_Word_t apif )
{
	L4_ApiFlags_t api;

	api.raw = apif;

	printf( "APIFlags reports %s endianess\n", endianess[api.x.ee] );
	printf( "APIFlags reports %s API width\n", apiwidth [api.x.ww] );
}

void
print_alignment( void *kip )
{
    L4_Word_t kipw = (L4_Word_t) kip;
    printf( "KIP alignment %s OK\n", 
	    (kipw == (kipw & PAGE_MASK)) ? "is" : "IS NOT");
}

void 
print_l4tag( L4_KernelInterfacePage_t *kip )
{
	char *tag = (char*) kip;
	printf( "L4tag is: %c%c%c%c\n", 
		tag[TAG0], tag[TAG1], tag[TAG2], tag[TAG3] );

	printf( "L4tag %s valid\n", 
		((*(L4_Word32_t*)tag) & 0xffffffff) == L4_MAGIC ? 
		"is" : "IS NOT" ); 
}

void print_infos( L4_KernelInterfacePage_t * kip )
{
    printf("Threads: IRQs=%ld, sys=%ld, valid TID bits=%ld\n", 
	   L4_ThreadIdSystemBase (kip),
	   L4_ThreadIdUserBase (kip) - L4_ThreadIdSystemBase (kip),
	   L4_ThreadIdBits (kip));
}

void print_kerndesc( L4_KernelInterfacePage_t * kip )
{
    L4_KernelDesc_t * desc;
    if (!kip->KernelVerPtr)
    {
	printf("KernelVerPtr is invalid (%p)\n", L4_KernelVersionString (kip));
	return;
    }

    desc = (L4_KernelDesc_t*)((L4_Word_t)kip + kip->KernelVerPtr);

    if (!check_kipptr(kip, desc))
	return;

    L4_Word_t year, month, day;
    L4_KernelGenDate (kip, &year, &month, &day);
    print_kernelid(desc->KernelId.raw);
    printf("Kernel generation date: %ld/%ld/%ld\n", day, month, year); 
    printf("Kernel Version: %ld.%ld.%ld\n", desc->KernelVer.X.ver,
	   desc->KernelVer.X.subver, desc->KernelVer.X.subsubver);
    char * sup = (char*)&desc->Supplier;
    printf("Supplier string: \"%c%c%c%c\"\n", sup[0], sup[1], sup[2], sup[3]);
    printf("Version string: \"%s\"\n", desc->VersionString);
}

void print_procdesc( L4_KernelInterfacePage_t * kip )
{
    int num = kip->ProcessorInfo.X.processors + 1;
    printf("Processors: %d\n", num);
    for (int cpu = 0; cpu < num; cpu++)
    {
	L4_ProcDesc_t * pdesc = L4_ProcDesc(kip, cpu);
	printf("  CPU%d: ", cpu);
	if ( check_kipptr(kip, pdesc) )
	    printf("int freq=%ldkHz, ext freq=%ldkHz\n",
		   L4_ProcDescInternalFreq (pdesc),
		   L4_ProcDescExternalFreq (pdesc));
	else
	    printf("invalid descriptor\n");
    }
}

static void
call_kip(int depth)
{
	void *kip;
	L4_Word_t apiv, apif, kid;

	if( depth == 0 )
		kip = L4_KernelInterface( &apiv, &apif, &kid );
	else
		call_kip( depth - 1 );
}


/* menu functions */

/* print out all the KIP info */
static void
print_kip(void)
{
	void *kip;
	L4_KernelInterfacePage_t *skip;
	L4_Word_t apiv, apif, kid;

	kip = L4_KernelInterface( &apiv, &apif, &kid );

	print_h1( "Kernel Interface Page" );

	/* dump what we got in registers */
	print_h2( "Register returned values");
	print_kernelid( kid );
	print_version( apiv );
	print_apiflags( apif );
	
	printf( "Address of KIP is %p\n", kip );
	print_alignment( kip );

	/* dump what we find in memory */
	print_h2( "KIP memory values");
	skip = (L4_KernelInterfacePage_t*) kip;
	print_l4tag( skip );

	print_version( skip->ApiVersion.raw );
	print_apiflags( skip->ApiFlags.raw );
	print_infos( skip );

	print_kerndesc( skip );
	print_procdesc( skip );
}

/* Get the KIP many times at different stack frame depths */
static void
thrash_kip(void)
{
	int i, j;

	for( j = 0; j < 10; j++ )
	{
		printf( "Checking KIP, depth %d\n", j );

		for( i = 0; i < 1000; i++ )
			call_kip(j);
	}
}

void all_kip_tests(void)
{
    print_kip();
    thrash_kip();
}

/* the menu */
static struct menuitem menu_items[] = 
{
	{ NULL, "return" },
	{ print_kip,  "Print KIP" },
	{ thrash_kip, "Thrash GetKip" },
        { all_kip_tests, "All KIP tests" },
};

static struct menu menu = 
{
	"KIP Menu",
	0, 
	NUM_ITEMS(menu_items),
	menu_items
};


/* entry point */
void 
kip_test(void)
{
	menu_input( &menu );
}

