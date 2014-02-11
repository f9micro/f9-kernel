/* Copyright (c) 1999-2004 University of New South Wales. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/**
\file

\brief Generic ELF library

The ELF library is designed to make the task of parsing and getting information
out of an ELF file easier.

It provides function to obtain the various different fields in the ELF header, and
the program and segment information.

Also importantly, it provides a function elf_loadFile which will load a given
ELF file into memory.

*/

#ifndef __ELF_ELF_H__
#define __ELF_ELF_H__

#include "elf32.h"
#include <debug.h>
#include <types.h>

/*
 * constants for Elf32_Phdr.p_flags
 */
#define PF_X		1	/* readable segment */
#define PF_W		2	/* writeable segment */
#define PF_R		4	/* executable segment */

/*
 * constants for indexing into Elf64_Header_t.e_ident
 */
#define EI_MAG0		0
#define EI_MAG1		1
#define EI_MAG2		2
#define EI_MAG3		3
#define EI_CLASS	4
#define EI_DATA		5
#define EI_VERSION	6

#define ELFMAG0         '\177'
#define ELFMAG1         'E'
#define ELFMAG2         'L'
#define ELFMAG3         'F'

#define ELFCLASS32      1
#define ELFCLASS64      2

#define PT_NULL 0
#define PT_LOAD 1
#define PT_DYNAMIC 2
#define PT_INTERP 3
#define PT_NOTE 4

#define ELFDATA2LSB 1
#define ELFDATA2MSB 2

/* Section Header type bits */
#define SHT_PROGBITS 1
#define SHT_SYMTAB 2
#define	SHT_NOBITS 8
#define SHT_REL 9

/* Section Header flag bits */
#define SHF_WRITE 1
#define SHF_ALLOC 2
#define SHF_EXECINSTR  4

/**/
#define ELF_PRINT_PROGRAM_HEADERS 1
#define ELF_PRINT_SECTIONS 2
#define ELF_PRINT_ALL (ELF_PRINT_PROGRAM_HEADERS | ELF_PRINT_SECTIONS)

/**
 * Checks that elfFile points to a valid elf file.
 *
 * @param elfFile Potential ELF file to check
 *
 * \return 0 on success. -1 if not and elf, -2 if not 32 bit.
 */
int elf_checkFile(void *elfFile);

/**
 * Determine number of sections in an ELF file.
 *
 * @param elfFile Pointer to a valid ELF header.
 *
 * \return Number of sections in the ELF file.
 */
unsigned elf_getNumSections(void *elfFile);

/**
 * Determine number of program headers in an ELF file.
 *
 * @param elfFile Pointer to a valid ELF header.
 *
 * \return Number of program headers in the ELF file.
 */
uint16_t elf_getNumProgramHeaders(void *elfFile);

/**
 * Return the base physical address of given program header in an ELF file
 *
 * @param elfFile Pointer to a valid ELF header
 * @param ph Index of the program header
 *
 * \return The memory size of the specified program header
 */
uint32_t elf_getProgramHeaderPaddr(void *elfFile, uint16_t ph);

/**
 * Return the base virtual address of given program header in an ELF file
 *
 * @param elfFile Pointer to a valid ELF header
 * @param ph Index of the program header
 *
 * \return The memory size of the specified program header
 */
uint32_t elf_getProgramHeaderVaddr(void *elfFile, uint16_t ph);

/**
 * Return the memory size of a given program header in an ELF file
 *
 * @param elfFile Pointer to a valid ELF header
 * @param ph Index of the program header
 *
 * \return The memory size of the specified program header
 */
uint32_t elf_getProgramHeaderMemorySize(void *elfFile, uint16_t ph);

/**
 * Return the file size of a given program header in an ELF file
 *
 * @param elfFile Pointer to a valid ELF header
 * @param ph Index of the program header
 *
 * \return The file size of the specified program header
 */
uint32_t elf_getProgramHeaderFileSize(void *elfFile, uint16_t ph);

/**
 * Return the start offset of he file
 *
 * @param elfFile Pointer to a valid ELF header
 * @param ph Index of the program header
 *
 * \return The offset of this program header with relation to the start
 * of the elfFile.
 */
uint32_t elf_getProgramHeaderOffset(void *elfFile, uint16_t ph);

/**
 * Return the flags for a given program header
 *
 * @param elfFile Pointer to a valid ELF header
 * @param ph Index of the program header
 *
 * \return The flags of a given program header
 */
uint32_t elf_getProgramHeaderFlags(void *elfFile, uint16_t ph);

/**
 * Return the type for a given program header
 *
 * @param elfFile Pointer to a valid ELF header
 * @param ph Index of the program header
 *
 * \return The type of a given program header
 */
uint32_t elf_getProgramHeaderType(void *elfFile, uint16_t ph);

/**
 * Return the physical translation of a physical address, with respect
 * to a given program header
 *
 */
uint32_t elf_vtopProgramHeader(void *elfFile, uint16_t ph, uint32_t vaddr);


/**
 *
 * \return true if the address in in this program header
 */
bool elf_vaddrInProgramHeader(void *elfFile, uint16_t ph, uint32_t vaddr);

/**
 * Determine the memory bounds of an ELF file
 *
 * @param elfFile Pointer to a valid ELF header
 * @param phys If true return bounds of physical memory, otherwise return
 *   bounds of virtual memory
 * @param min Pointer to return value of the minimum
 * @param max Pointer to return value of the maximum
 *
 * \return true on success. false on failure, if for example, it is an invalid ELF file
 */
bool elf_getMemoryBounds(void *elfFile, bool phys, uint32_t *min, uint32_t *max);

/**
 * Find the entry point of an ELF file.
 *
 * @param elfFile Pointer to a valid ELF header
 *
 * \return The entry point address as a 64-bit integer.
 */
uint32_t elf_getEntryPoint(void *elfFile);

/**
 * Load an ELF file into memory
 *
 * @param elfFile Pointer to a valid ELF file
 * @param phys If true load using the physical address, otherwise using the virtual addresses
 *
 * \return true on success, false on failure.
 *
 * The function assumes that the ELF file is loaded in memory at some
 * address different to the target address at which it will be loaded.
 * It also assumes direct access to the source and destination address, i.e:
 * Memory must be ale to me loaded with a simple memcpy.
 *
 * Obviously this also means that if we are loading a 64bit ELF on a 32bit
 * platform, we assume that any memory address are within the first 4GB.
 *
 */
uint32_t elf_loadFile(void *elfFile);

char *elf_getStringTable(void *elfFile, int string_segment);
char *elf_getSegmentStringTable(void *elfFile);
void *elf_getSectionNamed(void *elfFile, char *str);
char *elf_getSectionName(void *elfFile, int i);
uint32_t elf_getSectionSize(void *elfFile, int i);
uint32_t elf_getSectionAddr(void *elfFile, int i);

/**
 * Return the flags for a given sections
 *
 * @param elfFile Pointer to a valid ELF header
 * @param i Index of the sections
 *
 * \return The flags of a given section
 */
uint32_t elf_getSectionFlags(void *elfFile, int i);

/**
 * Return the type for a given sections
 *
 * @param elfFile Pointer to a valid ELF header
 * @param i Index of the sections
 *
 * \return The type of a given section
 */
uint32_t elf_getSectionType(void *elfFile, int i);

void *elf_getSection(void *elfFile, int i);
void elf_getProgramHeaderInfo(void *elfFile, uint16_t ph, uint32_t *p_vaddr,
			      uint32_t *p_paddr, uint32_t *p_filesz,
			      uint32_t *p_offset, uint32_t *p_memsz);


/**
 * output the details of an ELF file to the stream f
 */
void elf_fprintf(void *elfFile, int size, const char *name, int flags);

#if 0
/*
 * Returns a pointer to the program segment table, which is an array of
 * ELF64_Phdr_t structs.  The size of the array can be found by calling
 * getNumProgramSegments.
 */
struct Elf32_Phdr *elf_getProgramSegmentTable(void *elfFile);
#endif
#if 0
/**
 * Returns a pointer to the program segment table, which is an array of
 * ELF64_Phdr_t structs.  The size of the array can be found by calling
 * getNumProgramSegments.
 */
struct Elf32_Shdr *elf_getSectionTable(void *elfFile);
#endif

#endif
