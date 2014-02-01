/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Copyright (c) 2004 University of New South Wales. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <elf/elf.h>
#include <lib/string.h>

int elf32_checkFile(struct Elf32_Header *file)
{
	if (file->e_ident[EI_MAG0] != ELFMAG0 ||
	    file->e_ident[EI_MAG1] != ELFMAG1 ||
	    file->e_ident[EI_MAG2] != ELFMAG2 ||
	    file->e_ident[EI_MAG3] != ELFMAG3)
		return -1;	/* not an elf file */
	if (file->e_ident[EI_CLASS] != ELFCLASS32)
		return -2;	/* not 32-bit file */
	return 0;		/* elf file looks OK */
}

/*
 * Returns the number of program segments in this elf file.
 */
unsigned elf32_getNumSections(struct Elf32_Header *elfFile)
{
	return elfFile->e_shnum;
}

char *elf32_getStringTable(struct Elf32_Header *elfFile)
{
	struct Elf32_Shdr *sections = elf32_getSectionTable(elfFile);
	return (char *) elfFile + sections[elfFile->e_shstrndx].sh_offset;
}

/* Returns a pointer to the program segment table, which is an array of
 * ELF32_Phdr_t structs.  The size of the array can be found by calling
 * getNumProgramSegments. */
struct Elf32_Phdr *elf32_getProgramSegmentTable(struct Elf32_Header *elfFile)
{
	struct Elf32_Header *fileHdr = elfFile;
	return (struct Elf32_Phdr *)(fileHdr->e_phoff + (long) elfFile);
}

/* Returns the number of program segments in this elf file. */
uint16_t elf32_getNumProgramHeaders(struct Elf32_Header *elfFile)
{
	struct Elf32_Header *fileHdr = elfFile;
	return fileHdr->e_phnum;
}

char *elf32_getSectionName(struct Elf32_Header *elfFile, int i)
{
	struct Elf32_Shdr *sections = elf32_getSectionTable(elfFile);
	char *str_table = elf32_getSegmentStringTable(elfFile);
	if (str_table == NULL) {
		return "<corrupted>";
	} else {
		return str_table + sections[i].sh_name;
	}
}

uint32_t elf32_getSectionSize(struct Elf32_Header *elfFile, int i)
{
	struct Elf32_Shdr *sections = elf32_getSectionTable(elfFile);
	return sections[i].sh_size;
}

uint32_t elf32_getSectionAddr(struct Elf32_Header *elfFile, int i)
{
	struct Elf32_Shdr *sections = elf32_getSectionTable(elfFile);
	return sections[i].sh_addr;
}

void *elf32_getSection(struct Elf32_Header *elfFile, int i)
{
	struct Elf32_Shdr *sections = elf32_getSectionTable(elfFile);
	return (char *) elfFile + sections[i].sh_offset;
}

void *elf32_getSectionNamed(struct Elf32_Header *elfFile, char *str)
{
	int numSections = elf32_getNumSections(elfFile);
	int i;
	for (i = 0; i < numSections; i++) {
		if (strcmp(str, elf32_getSectionName(elfFile, i)) == 0) {
			return elf32_getSection(elfFile, i);
		}
	}
	return NULL;
}

char *elf32_getSegmentStringTable(struct Elf32_Header *elfFile)
{
	struct Elf32_Header *fileHdr = (struct Elf32_Header *) elfFile;
	if (fileHdr->e_shstrndx == 0) {
		return NULL;
	} else {
		return elf32_getStringTable(elfFile);
	}
}

#ifdef ELF_DEBUG
void elf32_printStringTable(struct Elf32_Header *elfFile)
{
	int counter;
	struct Elf32_Shdr *sections = elf32_getSectionTable(elfFile);
	char * stringTable;

	if (!sections) {
		dbg_printf(DL_BASIC, "No sections.\n");
		return;
	}

	stringTable = ((void *) elfFile) + sections[elfFile->e_shstrndx].sh_offset;

	dbg_printf(DL_BASIC, "File is %p; sections is %p; string table is %p\n", elfFile, sections, stringTable);

	for (counter = 0; counter < sections[elfFile->e_shstrndx].sh_size; counter++) {
		dbg_printf(DL_BASIC, "%02x %c ", stringTable[counter],
		           stringTable[counter] >= 0x20 ? stringTable[counter] : '.');
	}
}
#endif

int elf32_getSegmentType(struct Elf32_Header *elfFile, int segment)
{
	return elf32_getProgramSegmentTable(elfFile)[segment].p_type;
}

void elf32_getSegmentInfo(struct Elf32_Header *elfFile, int segment,
                          uint64_t *p_vaddr, uint64_t *p_addr,
                          uint64_t *p_filesz, uint64_t *p_offset,
                          uint64_t *p_memsz)
{
	struct Elf32_Phdr *segments;

	segments = elf32_getProgramSegmentTable(elfFile);
	*p_addr = segments[segment].p_paddr;
	*p_vaddr = segments[segment].p_vaddr;
	*p_filesz = segments[segment].p_filesz;
	*p_offset = segments[segment].p_offset;
	*p_memsz = segments[segment].p_memsz;
}

uint32_t elf32_getEntryPoint(struct Elf32_Header *elfFile)
{
	return elfFile->e_entry;
}
