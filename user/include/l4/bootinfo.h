/*********************************************************************
 *                
 * Copyright (C) 2004, 2007,  Karlsruhe University
 *                
 * File path:     l4/bootinfo.h
 * Description:   The L4 common bootinfo interface
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
 * $Id: bootinfo.h,v 1.7 2004/07/06 10:03:57 benno Exp $
 *                
 ********************************************************************/
#ifndef __L4__BOOTINFO_H__
#define __L4__BOOTINFO_H__

#include <l4/types.h>


/*
 * The L4 bootinfo magic.  The magic also indicates the endianess of
 * the bootinfo structure.  The word size is defined to be the same as
 * specified in the kernel interface page.
 */
#define L4_BOOTINFO_MAGIC		((L4_Word_t) 0x14b0021d)


/*
 * Current version of the L4 bootinfo structure itself.  Note that
 * only changes to the bootinfo structure itself warrant an increase
 * in the version number.  A change in a bootinfo record structure
 * will only increase the version number of that particular type.
 */
#define L4_BOOTINFO_VERSION		1


/*
 * Currently defined bootinfo record types.
 */
#define L4_BootInfo_Module			0x0001
#define L4_BootInfo_SimpleExec			0x0002
#define L4_BootInfo_EFITables			0x0101
#define L4_BootInfo_Multiboot			0x0102



/*
 * The BootRec class is just the generic part of the the boot record
 * class.
 */
typedef struct {
    L4_Word_t	type;
    L4_Word_t	version;
    L4_Word_t	offset_next;
} L4_BootRec_t;

L4_INLINE L4_Word_t L4_BootRec_Type (const L4_BootRec_t * r)
{
    return r->type;
}

L4_INLINE L4_BootRec_t * L4_BootRec_Next (const L4_BootRec_t * r)
{
    return (L4_BootRec_t *) ((L4_Word8_t *) r + r->offset_next);
}

#if defined(__cplusplus)
L4_INLINE L4_Word_t L4_Type (const L4_BootRec_t * r)
{
    return L4_BootRec_Type (r);
}

L4_INLINE L4_BootRec_t * L4_Next (const L4_BootRec_t * r)
{
    return L4_BootRec_Next (r);
}
#endif



/*
 * Bootinfo type:	Module
 * 
 * A Module describes a binary file inserted into memory by the
 * bootloader.
 */
typedef struct {
    L4_Word_t	type;			// 0x01
    L4_Word_t	version;		// 1
    L4_Word_t	offset_next;

    L4_Word_t	start;
    L4_Word_t	size;
    L4_Word_t	cmdline_offset;
} L4_Boot_Module_t;

L4_INLINE L4_Word_t L4_Module_Start (const L4_BootRec_t * ptr)
{
    L4_Boot_Module_t * m = (L4_Boot_Module_t *) ptr;
    return m->start;
}

L4_INLINE L4_Word_t L4_Module_Size (const L4_BootRec_t * ptr)
{
    L4_Boot_Module_t * m = (L4_Boot_Module_t *) ptr;
    return m->size;
}

L4_INLINE char * L4_Module_Cmdline (const L4_BootRec_t * ptr)
{
    L4_Boot_Module_t * m = (L4_Boot_Module_t *) ptr;
    if (m->cmdline_offset)
	return (char *) m + m->cmdline_offset;
    else
	return (char *) 0;
}



/*
 * Bootinfo type:	SimpleExec
 * 
 * A SimpleExec describes an executable binary decoded and relocated
 * into memory by the bootloader
 */
typedef struct {
    L4_Word_t	type;			// 0x02
    L4_Word_t	version;		// 1
    L4_Word_t	offset_next;

    L4_Word_t	text_pstart;
    L4_Word_t	text_vstart;
    L4_Word_t	text_size;
    L4_Word_t	data_pstart;
    L4_Word_t	data_vstart;
    L4_Word_t	data_size;
    L4_Word_t	bss_pstart;
    L4_Word_t	bss_vstart;
    L4_Word_t	bss_size;
    L4_Word_t	initial_ip;
    L4_Word_t	flags;
    L4_Word_t	label;
    L4_Word_t	cmdline_offset;
} L4_Boot_SimpleExec_t;

L4_INLINE L4_Word_t L4_SimpleExec_TextVstart (const L4_BootRec_t * ptr)
{
    L4_Boot_SimpleExec_t * e = (L4_Boot_SimpleExec_t *) ptr;
    return e->text_vstart;
}

L4_INLINE L4_Word_t L4_SimpleExec_TextPstart (const L4_BootRec_t * ptr)
{
    L4_Boot_SimpleExec_t * e = (L4_Boot_SimpleExec_t *) ptr;
    return e->text_pstart;
}

L4_INLINE L4_Word_t L4_SimpleExec_TextSize (const L4_BootRec_t * ptr)
{
    L4_Boot_SimpleExec_t * e = (L4_Boot_SimpleExec_t *) ptr;
    return e->text_size;
}

L4_INLINE L4_Word_t L4_SimpleExec_DataVstart (const L4_BootRec_t * ptr)
{
    L4_Boot_SimpleExec_t * e = (L4_Boot_SimpleExec_t *) ptr;
    return e->data_vstart;
}

L4_INLINE L4_Word_t L4_SimpleExec_DataPstart (const L4_BootRec_t * ptr)
{
    L4_Boot_SimpleExec_t * e = (L4_Boot_SimpleExec_t *) ptr;
    return e->data_pstart;
}

L4_INLINE L4_Word_t L4_SimpleExec_DataSize (const L4_BootRec_t * ptr)
{
    L4_Boot_SimpleExec_t * e = (L4_Boot_SimpleExec_t *) ptr;
    return e->data_size;
}

L4_INLINE L4_Word_t L4_SimpleExec_BssVstart (const L4_BootRec_t * ptr)
{
    L4_Boot_SimpleExec_t * e = (L4_Boot_SimpleExec_t *) ptr;
    return e->bss_vstart;
}

L4_INLINE L4_Word_t L4_SimpleExec_BssPstart (const L4_BootRec_t * ptr)
{
    L4_Boot_SimpleExec_t * e = (L4_Boot_SimpleExec_t *) ptr;
    return e->bss_pstart;
}

L4_INLINE L4_Word_t L4_SimpleExec_BssSize (const L4_BootRec_t * ptr)
{
    L4_Boot_SimpleExec_t * e = (L4_Boot_SimpleExec_t *) ptr;
    return e->bss_size;
}

L4_INLINE L4_Word_t L4_SimpleExec_InitialIP (const L4_BootRec_t * ptr)
{
    L4_Boot_SimpleExec_t * e = (L4_Boot_SimpleExec_t *) ptr;
    return e->initial_ip;
}

L4_INLINE L4_Word_t L4_SimpleExec_Flags (const L4_BootRec_t * ptr)
{
    L4_Boot_SimpleExec_t * e = (L4_Boot_SimpleExec_t *) ptr;
    return e->flags;
}

L4_INLINE void L4_SimpleExec_Set_Flags (const L4_BootRec_t * ptr, L4_Word_t w)
{
    L4_Boot_SimpleExec_t * e = (L4_Boot_SimpleExec_t *) ptr;
    e->flags = w;
}

L4_INLINE L4_Word_t L4_SimpleExec_Label (const L4_BootRec_t * ptr)
{
    L4_Boot_SimpleExec_t * e = (L4_Boot_SimpleExec_t *) ptr;
    return e->label;
}

L4_INLINE void L4_SimpleExec_Set_Label (const L4_BootRec_t * ptr, L4_Word_t w)
{
    L4_Boot_SimpleExec_t * e = (L4_Boot_SimpleExec_t *) ptr;
    e->label = w;
}

L4_INLINE char * L4_SimpleExec_Cmdline (const L4_BootRec_t * ptr)
{
    L4_Boot_SimpleExec_t * e = (L4_Boot_SimpleExec_t *) ptr;
    if (e->cmdline_offset)
	return (char *) ((L4_Word8_t *) e + e->cmdline_offset);
    else
	return (char *) 0;
}



/*
 * Bootinfo type:	EFI
 * 
 * An EFI record describes location and size of tables for the
 * Extensible Firmware Interface.
 */
typedef struct {
    L4_Word_t	type;			// 0x101
    L4_Word_t	version;		// 1
    L4_Word_t	offset_next;

    L4_Word_t	systab;
    L4_Word_t	memmap;
    L4_Word_t	memmap_size;
    L4_Word_t	memdesc_size;
    L4_Word_t	memdesc_version;
} L4_Boot_EFI_t;

L4_INLINE L4_Word_t L4_EFI_Systab (const L4_BootRec_t * ptr)
{
    L4_Boot_EFI_t * e = (L4_Boot_EFI_t *) ptr;
    return e->systab;
}

L4_INLINE L4_Word_t L4_EFI_Memmap (const L4_BootRec_t * ptr)
{
    L4_Boot_EFI_t * e = (L4_Boot_EFI_t *) ptr;
    return e->memmap;
}

L4_INLINE L4_Word_t L4_EFI_MemmapSize (const L4_BootRec_t * ptr)
{
    L4_Boot_EFI_t * e = (L4_Boot_EFI_t *) ptr;
    return e->memmap_size;
}

L4_INLINE L4_Word_t L4_EFI_MemdescSize (const L4_BootRec_t * ptr)
{
    L4_Boot_EFI_t * e = (L4_Boot_EFI_t *) ptr;
    return e->memdesc_size;
}

L4_INLINE L4_Word_t L4_EFI_MemdescVersion (const L4_BootRec_t * ptr)
{
    L4_Boot_EFI_t * e = (L4_Boot_EFI_t *) ptr;
    return e->memdesc_version;
}



/*
 * Bootinfo type:	MBI
 * 
 * An MBI record describes location of a multiboot info structure.
 */
typedef struct {
    L4_Word_t	type;			// 0x102
    L4_Word_t	version;		// 1
    L4_Word_t	offset_next;

    L4_Word_t	address;
} L4_Boot_MBI_t;

L4_INLINE L4_Word_t L4_MBI_Address (const L4_BootRec_t * ptr)
{
    L4_Boot_MBI_t * m = (L4_Boot_MBI_t *) ptr;
    return m->address;
}


/*
 * The L4 bootinfo structure.
 */
typedef struct {
    L4_Word_t	magic;
    L4_Word_t	version;
    L4_Word_t	size;
    L4_Word_t	first_entry;
    L4_Word_t	num_entries;
    L4_Word_t	__reserved[3];
} L4_BootInfo_t;

L4_INLINE L4_Bool_t L4_BootInfo_Valid (const void * ptr)
{
    L4_BootInfo_t * b = (L4_BootInfo_t *) ptr;
    return (b->magic == L4_BOOTINFO_MAGIC) &&
	(b->version == L4_BOOTINFO_VERSION);
}

L4_INLINE L4_Word_t L4_BootInfo_Size (const void * ptr)
{
    L4_BootInfo_t * b = (L4_BootInfo_t *) ptr;
    return b->size;
}

L4_INLINE L4_BootRec_t * L4_BootInfo_FirstEntry (const void * ptr)
{
    L4_BootInfo_t * b = (L4_BootInfo_t *) ptr;
    return (L4_BootRec_t *) ((L4_Word8_t *) b + b->first_entry);
}

L4_INLINE L4_Word_t L4_BootInfo_Entries (const void * ptr)
{
    L4_BootInfo_t * b = (L4_BootInfo_t *) ptr;
    return b->num_entries;
}



#endif /* !__L4__BOOTINFO_H__ */
