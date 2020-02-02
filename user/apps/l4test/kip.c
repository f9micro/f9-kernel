/* Copyright (c) 2002, 2003, 2007, 2010 Karlsruhe University.
 * All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <l4/ipc.h>
#include <l4/kip.h>
#include <l4/space.h>
#include <l4/thread.h>
#include <l4/arch.h>
#include <l4/kdebug.h>
#include <l4/pager.h>
#include <l4io.h>
#include <platform/cortex_m.h>

#include "arch.h"
#include "config.h"
#include "l4test.h"
#include "assert.h"

__USER_TEXT
int check_kipptr(L4_KernelInterfacePage_t *kip, void * ptr)
{
	if (((L4_Word_t)kip + (1 << kip->KipAreaInfo.X.s) >= (L4_Word_t)ptr) 
		&& (ptr >= (void *) kip))
		return 1;
	printf("ERROR: kip-ptr (%p) not within KIP area\n", ptr);
	return 0;
}

/* list of kernel ids, subids and what they are */
typedef struct kid_list_t {
	L4_Word_t id;
	L4_Word_t subid;
	const char *kernel;
	const char * supplier;
} kid_list_t;

__USER_DATA
kid_list_t kid_list[] = 
{
	{ 0, 0, "F9", "NCKU" },
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
typedef struct api_list_t {
	L4_Word_t version;
	L4_Word_t subversion;
	const char *api;
} api_list_t;

__USER_DATA
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
__USER_DATA const char *endianess[] = {"little", "big", "ERROR-2", "ERROR-3"};
__USER_DATA const char *apiwidth [] = {"32-bit", "64-bit", "ERROR-2", "ERROR-3"};

#define TAG0 0
#define TAG1 1
#define TAG2 2
#define TAG3 3

/* other module functions */
__USER_TEXT
void print_kernelid(L4_Word_t kid_w)
{
	L4_KernelId_t kid;
	L4_Word_t i;
	const char *kernel, *supplier;

	kid.raw = kid_w;

	for (i = 0; i < KID_LIST_COUNT; ++i) {
		if ((kid_list[i].id == kid.x.id) &&
			(kid_list[i].subid == kid.x.subid)) {
			break;
		}
	}

	if (i != KID_LIST_COUNT) {
		kernel = kid_list[i].kernel;
		supplier = kid_list[i].supplier;
	} else {
		kernel = "unknown kernel";
		supplier = "unknow supplier";
	}

	printf("KernelID reports 0x%x.0x%x: %s from %s\n",
		   (long) kid.x.id, (long) kid.x.subid,
		   kernel, supplier);
}

__USER_TEXT
void print_version(L4_Word_t apiv)
{
	L4_ApiVersion_t api;
	L4_Word_t i;
	const char *version;

	api.raw = apiv;

	for (i = 0; i < API_LIST_COUNT; ++i) {
		if ((api.x.version == api_list[i].version) &&
			((api.x.subversion == api_list[i].subversion) ||
			 (api_list[i].subversion == NO_SUBVERSION))) {
			break;
		}
	}

	if (i != API_LIST_COUNT) {
		version = api_list[i].api;
	} else {
		version = "unknow api";
	}

	printf("APIVersion reports %d.%d: %s\n",
		   (int) api.x.version, (int) api.x.subversion,
		   version);
}

__USER_TEXT
void print_apiflags(L4_Word_t apif)
{
	L4_ApiFlags_t api;

	api.raw = apif;

	printf("APIFlags reports %s endianess\n", endianess[api.x.ee]);
	printf("APIFlags reports %s API width\n", apiwidth [api.x.ww]);
}


__USER_TEXT
void print_alignment(void *kip)
{
	L4_Word_t kipw = (L4_Word_t) kip;
	printf("KIP alignment %s OK\n",
		(kipw == (kipw & PAGE_MASK)) ? "is" : "IS NOT");
}

__USER_TEXT
void print_l4tag(L4_KernelInterfacePage_t *kip)
{
	char *tag = (char *) kip;
	printf("L4tag is: %c%c%c%c\n", 
		tag[TAG0], tag[TAG1], tag[TAG2], tag[TAG3]);

	printf( "L4tag %s valid\n", 
		((*(L4_Word32_t*)tag) & 0xffffffff) == L4_MAGIC ? 
		"is" : "IS NOT"); 
}

__USER_TEXT
void print_infos(L4_KernelInterfacePage_t * kip)
{
	printf("Threads: IRQs=%ld, sys=%ld, valid TID bits=%ld\n", 
	   L4_ThreadIdSystemBase(kip),
	   L4_ThreadIdUserBase(kip) - L4_ThreadIdSystemBase(kip),
	   L4_ThreadIdBits(kip));
}

__USER_TEXT
void print_kerndesc( L4_KernelInterfacePage_t * kip )
{
	L4_KernelDesc_t * desc;
	if (!kip->KernelVerPtr) {
		printf("KernelVerPtr is invalid (%p)\n", L4_KernelVersionString (kip));
		return;
	}

	/* FIXME: Not test yet */
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

__USER_TEXT
void print_procdesc(L4_KernelInterfacePage_t * kip)
{
	int num = kip->ProcessorInfo.X.processors + 1;
	printf("Processors: %d\n", num);
	for (int cpu = 0; cpu < num; cpu++) {
		L4_ProcDesc_t *pdesc = L4_ProcDesc(kip, cpu);
		printf("  CPU%d: ", cpu);
		if (check_kipptr(kip, pdesc)) {
			printf("int freq=%ldkHz, ext freq=%ldkHz\n",
					L4_ProcDescInternalFreq (pdesc),
					L4_ProcDescExternalFreq (pdesc));
		} else {
			printf("invalid descriptor\n");
		}
	}
}

__USER_TEXT
static void print_kip(void)
{
	void *kip;
	L4_KernelInterfacePage_t *skip;
	L4_Word_t apiv, apif, kid;

	kip = L4_KernelInterface(&apiv, &apif, &kid);
	print_h1("Kernel Interface Page");

	/* dump what we got in registers */
	print_kernelid(kid);
	print_version(apiv);
	print_apiflags(apif);

	printf("Address of KIP is %p\n", kip);
	print_alignment(kip);

	/* dump what we find in memory */
	print_h2("KIP memory values");
	skip = (L4_KernelInterfacePage_t *) kip;
	print_l4tag(skip);

	print_version(skip->ApiVersion.raw);
	print_apiflags(skip->ApiFlags.raw);
	print_infos(skip);

	print_kerndesc(skip); /* Unimplemented descriptor */
	print_procdesc(skip); /* Unimplemented descriptor */
}

__USER_TEXT
static void call_kip(int depth)
{
	void *kip;
	L4_Word_t apiv, apif, kid;

	if(depth == 0)
		kip = L4_KernelInterface(&apiv, &apif, &kid);
	else
		call_kip( depth - 1 );

	/* Because our compiler told us we set `kip` but not used*/
	kip = kip;
}

__USER_TEXT
static void thrash_kip(void)
{
	int i, j;

	for(j = 0; j < 10; j++) {
		printf("Checking KIP, depth %d\n", j);

		for(i = 0; i < 1000; i++)
			call_kip(j);
	}
}

__USER_TEXT
void all_kip_tests(void)
{
	print_kip();
	thrash_kip();
}
