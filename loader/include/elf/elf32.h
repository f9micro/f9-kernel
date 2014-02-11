/* Copyright (c) 2004 University of New South Wales. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __ELF_ELF_32_H__
#define __ELF_ELF_32_H__

#include <stdint.h>

/*
 * File header
 */
struct Elf32_Header {
	unsigned char   e_ident[16];
	uint16_t        e_type;	/* Relocatable=1, Executable=2 (+ some
				 * more ..) */
	uint16_t        e_machine;	/* Target architecture: MIPS=8 */
	uint32_t        e_version;	/* Elf version (should be 1) */
	uint32_t        e_entry;	/* Code entry point */
	uint32_t        e_phoff;	/* Program header table */
	uint32_t        e_shoff;	/* Section header table */
	uint32_t        e_flags;	/* Flags */
	uint16_t        e_ehsize;	/* ELF header size */
	uint16_t        e_phentsize;	/* Size of one program segment
					 * header */
	uint16_t        e_phnum;	/* Number of program segment
					 * headers */
	uint16_t        e_shentsize;	/* Size of one section header */
	uint16_t        e_shnum;	/* Number of section headers */
	uint16_t        e_shstrndx;	/* Section header index of the
					 * string table for section header
					 * * names */
};

/*
 * Section header
 */
struct Elf32_Shdr {
	uint32_t        sh_name;
	uint32_t        sh_type;
	uint32_t        sh_flags;
	uint32_t        sh_addr;
	uint32_t        sh_offset;
	uint32_t        sh_size;
	uint32_t        sh_link;
	uint32_t        sh_info;
	uint32_t        sh_addralign;
	uint32_t        sh_entsize;
};

/*
 * Program header
 */
struct Elf32_Phdr {
	uint32_t p_type;	/* Segment type: Loadable segment = 1 */
	uint32_t p_offset;	/* Offset of segment in file */
	uint32_t p_vaddr;	/* Reqd virtual address of segment
					 * when loading */
	uint32_t p_paddr;	/* Reqd physical address of
					 * segment (ignore) */
	uint32_t p_filesz;	/* How many bytes this segment
					 * occupies in file */
	uint32_t p_memsz;	/* How many bytes this segment
					 * should occupy in * memory (when
					 * * loading, expand the segment
					 * by * concatenating enough zero
					 * bytes to it) */
	uint32_t p_flags;	/* Flags: logical "or" of PF_
					 * constants below */
	uint32_t p_align;	/* Reqd alignment of segment in
					 * memory */
};

int elf32_checkFile(struct Elf32_Header *file);
struct Elf32_Phdr *elf32_getProgramSegmentTable(struct Elf32_Header *file);
unsigned elf32_getNumSections(struct Elf32_Header *file);
char *elf32_getStringTable(struct Elf32_Header *file);
char *elf32_getSegmentStringTable(struct Elf32_Header *file);

static inline struct Elf32_Shdr *
elf32_getSectionTable(struct Elf32_Header *file)
{
	/* Cast heaven! */
	return (struct Elf32_Shdr *) (uintptr_t) (((uintptr_t) file) + file->e_shoff);
}

/* accessor functions */
static inline uint32_t
elf32_getSectionType(struct Elf32_Header *file, uint16_t s)
{
	return elf32_getSectionTable(file)[s].sh_type;
}

static inline uint32_t
elf32_getSectionFlags(struct Elf32_Header *file, uint16_t s)
{
	return elf32_getSectionTable(file)[s].sh_flags;
}

char *elf32_getSectionName(struct Elf32_Header *file, int i);
uint32_t elf32_getSectionSize(struct Elf32_Header *file, int i);
uint32_t elf32_getSectionAddr(struct Elf32_Header *elfFile, int i);
void *elf32_getSection(struct Elf32_Header *file, int i);
void *elf32_getSectionNamed(struct Elf32_Header *file, char *str);
int elf32_getSegmentType (struct Elf32_Header *file, int segment);
void elf32_getSegmentInfo(struct Elf32_Header *file, int segment, uint64_t *p_vaddr,
			  uint64_t *p_paddr, uint64_t *p_filesz,
			  uint64_t *p_offset, uint64_t *p_memsz);



uint32_t elf32_getEntryPoint (struct Elf32_Header *file);

/* Program header functions */
uint16_t elf32_getNumProgramHeaders(struct Elf32_Header *file);

static inline struct Elf32_Phdr *
elf32_getProgramHeaderTable(struct Elf32_Header *file)
{
	/* Cast heaven! */
	return (struct Elf32_Phdr*) (uintptr_t) (((uintptr_t) file) + file->e_phoff);
}

/* accessor functions */
static inline uint32_t
elf32_getProgramHeaderFlags(struct Elf32_Header *file, uint16_t ph)
{
	return elf32_getProgramHeaderTable(file)[ph].p_flags;
}

static inline uint32_t
elf32_getProgramHeaderType(struct Elf32_Header *file, uint16_t ph)
{
	return elf32_getProgramHeaderTable(file)[ph].p_type;
}

static inline uint32_t
elf32_getProgramHeaderFileSize(struct Elf32_Header *file, uint16_t ph)
{
	return elf32_getProgramHeaderTable(file)[ph].p_filesz;
}

static inline uint32_t
elf32_getProgramHeaderMemorySize(struct Elf32_Header *file, uint16_t ph)
{
	return elf32_getProgramHeaderTable(file)[ph].p_memsz;
}

static inline uint32_t
elf32_getProgramHeaderVaddr(struct Elf32_Header *file, uint16_t ph)
{
	return elf32_getProgramHeaderTable(file)[ph].p_vaddr;
}

static inline uint32_t
elf32_getProgramHeaderPaddr(struct Elf32_Header *file, uint16_t ph)
{
	return elf32_getProgramHeaderTable(file)[ph].p_paddr;
}

static inline uint32_t
elf32_getProgramHeaderOffset(struct Elf32_Header *file, uint16_t ph)
{
	return elf32_getProgramHeaderTable(file)[ph].p_offset;
}

#endif /* __ELF_ELF_32_H__ */
