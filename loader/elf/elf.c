/*
 * Australian Public Licence B (OZPLB)
 * 
 * Version 1-0
 * 
 * Copyright (c) 2004 University of New South Wales
 * 
 * All rights reserved. 
 * 
 * Developed by: Operating Systems and Distributed Systems Group (DiSy)
 *               University of New South Wales
 *               http://www.disy.cse.unsw.edu.au
 * 
 * Permission is granted by University of New South Wales, free of charge, to
 * any person obtaining a copy of this software and any associated
 * documentation files (the "Software") to deal with the Software without
 * restriction, including (without limitation) the rights to use, copy,
 * modify, adapt, merge, publish, distribute, communicate to the public,
 * sublicense, and/or sell, lend or rent out copies of the Software, and
 * to permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 * 
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimers.
 * 
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimers in the documentation and/or other materials provided
 *       with the distribution.
 * 
 *     * Neither the name of University of New South Wales, nor the names of its
 *       contributors, may be used to endorse or promote products derived
 *       from this Software without specific prior written permission.
 * 
 * EXCEPT AS EXPRESSLY STATED IN THIS LICENCE AND TO THE FULL EXTENT
 * PERMITTED BY APPLICABLE LAW, THE SOFTWARE IS PROVIDED "AS-IS", AND
 * NATIONAL ICT AUSTRALIA AND ITS CONTRIBUTORS MAKE NO REPRESENTATIONS,
 * WARRANTIES OR CONDITIONS OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO ANY REPRESENTATIONS, WARRANTIES OR CONDITIONS
 * REGARDING THE CONTENTS OR ACCURACY OF THE SOFTWARE, OR OF TITLE,
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NONINFRINGEMENT,
 * THE ABSENCE OF LATENT OR OTHER DEFECTS, OR THE PRESENCE OR ABSENCE OF
 * ERRORS, WHETHER OR NOT DISCOVERABLE.
 * 
 * TO THE FULL EXTENT PERMITTED BY APPLICABLE LAW, IN NO EVENT SHALL
 * NATIONAL ICT AUSTRALIA OR ITS CONTRIBUTORS BE LIABLE ON ANY LEGAL
 * THEORY (INCLUDING, WITHOUT LIMITATION, IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHERWISE) FOR ANY CLAIM, LOSS, DAMAGES OR OTHER
 * LIABILITY, INCLUDING (WITHOUT LIMITATION) LOSS OF PRODUCTION OR
 * OPERATION TIME, LOSS, DAMAGE OR CORRUPTION OF DATA OR RECORDS; OR LOSS
 * OF ANTICIPATED SAVINGS, OPPORTUNITY, REVENUE, PROFIT OR GOODWILL, OR
 * OTHER ECONOMIC LOSS; OR ANY SPECIAL, INCIDENTAL, INDIRECT,
 * CONSEQUENTIAL, PUNITIVE OR EXEMPLARY DAMAGES, ARISING OUT OF OR IN
 * CONNECTION WITH THIS LICENCE, THE SOFTWARE OR THE USE OF OR OTHER
 * DEALINGS WITH THE SOFTWARE, EVEN IF NATIONAL ICT AUSTRALIA OR ITS
 * CONTRIBUTORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH CLAIM, LOSS,
 * DAMAGES OR OTHER LIABILITY.
 * 
 * If applicable legislation implies representations, warranties, or
 * conditions, or imposes obligations or liability on University of New South
 * Wales or one of its contributors in respect of the Software that
 * cannot be wholly or partly excluded, restricted or modified, the
 * liability of University of New South Wales or the contributor is limited, to
 * the full extent permitted by the applicable legislation, at its
 * option, to:
 * a.  in the case of goods, any one or more of the following:
 * i.  the replacement of the goods or the supply of equivalent goods;
 * ii.  the repair of the goods;
 * iii. the payment of the cost of replacing the goods or of acquiring
 *  equivalent goods;
 * iv.  the payment of the cost of having the goods repaired; or
 * b.  in the case of services:
 * i.  the supplying of the services again; or
 * ii.  the payment of the cost of having the services supplied again.
 * 
 * The construction, validity and performance of this licence is governed
 * by the laws in force in New South Wales, Australia.
 */
void *str;
#include <elf/elf.h>
#include <string.h>

/*
 * Checks that elfFile points to a valid elf file. Returns 0 if the elf
 * file is valid, < 0 if invalid. 
 */

int
elf_checkFile(void *elfFile)
{
	return elf32_checkFile(elfFile);
}

/* Program Headers Access functions */
uint16_t
elf_getNumProgramHeaders(void *elfFile)
{
	return elf32_getNumProgramHeaders(elfFile);
}

uint32_t
elf_getProgramHeaderFlags(void *elfFile, uint16_t ph)
{
	return elf32_getProgramHeaderFlags(elfFile, ph);
}

uint32_t
elf_getProgramHeaderType(void *elfFile, uint16_t ph)
{
	return elf32_getProgramHeaderType(elfFile, ph);
}

uint32_t
elf_getProgramHeaderPaddr(void *elfFile, uint16_t ph)
{
	return elf32_getProgramHeaderPaddr(elfFile, ph);
}

uint32_t
elf_getProgramHeaderVaddr(void *elfFile, uint16_t ph)
{
	return elf32_getProgramHeaderVaddr(elfFile, ph);
}

uint32_t
elf_getProgramHeaderMemorySize(void *elfFile, uint16_t ph)
{
	return elf32_getProgramHeaderMemorySize(elfFile, ph);
}

uint32_t
elf_getProgramHeaderFileSize(void *elfFile, uint16_t ph)
{
	return elf32_getProgramHeaderFileSize(elfFile, ph);
}

uint32_t
elf_getProgramHeaderOffset(void *elfFile, uint16_t ph)
{
	return elf32_getProgramHeaderOffset(elfFile, ph);
}

char *
elf_getSegmentStringTable(void *elfFile)
{
	return elf32_getSegmentStringTable(elfFile);
}

char *
elf_getStringTable(void *elfFile, int string_segment)
{
	return elf32_getStringTable(elfFile);
}


unsigned
elf_getNumSections(void *elfFile)
{
	return elf32_getNumSections(elfFile);
}

char *
elf_getSectionName(void *elfFile, int i)
{
	return elf32_getSectionName(elfFile, i);
}

uint32_t
elf_getSectionFlags(void *elfFile, int i)
{
	return elf32_getSectionFlags(elfFile, i);
}

uint32_t
elf_getSectionType(void *elfFile, int i)
{
	return elf32_getSectionType(elfFile, i);
}

uint32_t
elf_getSectionSize(void *elfFile, int i)
{
	return elf32_getSectionSize(elfFile, i);
}

uint32_t
elf_getSectionAddr(void *elfFile, int i)
{
	return elf32_getSectionAddr(elfFile, i);
}


void *
elf_getSection(void *elfFile, int i)
{
	return elf32_getSection(elfFile, i);
}

void *
elf_getSectionNamed(void *elfFile, char *_str)
{
	return elf32_getSectionNamed(elfFile, _str);
}

void
elf_getProgramHeaderInfo(void *elfFile, uint16_t ph, uint32_t *p_vaddr, 
			 uint32_t *p_paddr, uint32_t *p_filesz, uint32_t *p_offset, 
			 uint32_t *p_memsz)
{
	*p_vaddr = elf_getProgramHeaderVaddr(elfFile, ph);
	*p_paddr = elf_getProgramHeaderPaddr(elfFile, ph);
	*p_filesz = elf_getProgramHeaderFileSize(elfFile, ph);
	*p_offset = elf_getProgramHeaderOffset(elfFile, ph);
	*p_memsz = elf_getProgramHeaderMemorySize(elfFile, ph);
}

uint32_t
elf_getEntryPoint(void *elfFile)
{
	return elf32_getEntryPoint(elfFile);
}


bool
elf_getMemoryBounds(void *elfFile, bool phys, uint32_t *min, uint32_t *max)
{
	uint32_t mem_min = UINT32_MAX;
	uint32_t mem_max = 0;
	int i;

	if (elf_checkFile(elfFile) != 0) {
		return false;
	}

	for(i=0; i < elf_getNumProgramHeaders(elfFile); i++) {
		uint32_t sect_min, sect_max;

		if (elf_getProgramHeaderMemorySize(elfFile, i) == 0) {
			continue;
		}

		if (phys) {
			sect_min = elf_getProgramHeaderPaddr(elfFile, i);
		} else {
			sect_min = elf_getProgramHeaderVaddr(elfFile, i);
		}

		sect_max = sect_min + elf_getProgramHeaderMemorySize(elfFile, i);

		if (sect_max > mem_max) {
			mem_max = sect_max;
		}
		if (sect_min < mem_min) {
			mem_min = sect_min;
		}
	}
	*min = mem_min;
	*max = mem_max;

	return true;
};

bool
elf_vaddrInProgramHeader(void *elfFile, uint16_t ph, uint32_t vaddr)
{
	uint32_t min = elf_getProgramHeaderVaddr(elfFile, ph);
	uint32_t max = min + elf_getProgramHeaderMemorySize(elfFile, ph);
	if (vaddr >= min && vaddr < max) {
		return true;
	} else {
		return false;
	}
}

uint32_t
elf_vtopProgramHeader(void *elfFile, uint16_t ph, uint32_t vaddr)
{
	uint32_t ph_phys = elf_getProgramHeaderPaddr(elfFile, ph);
	uint32_t ph_virt = elf_getProgramHeaderVaddr(elfFile, ph);
	uint32_t paddr;

	paddr = vaddr - ph_virt + ph_phys;

	return paddr;
}

uint32_t elf_loadFile(void *elfFile)
{
	int i;
	int num_pheaders;
	if (elf_checkFile(elfFile) != 0) {
		return false;
	}

	num_pheaders = elf_getNumProgramHeaders(elfFile);
	dbg_printf(DL_BASIC, "Number of program headers: %d\n", num_pheaders);

	for(i=0; i < num_pheaders; i++) {
		/* Load that section */
		uint32_t dest, src;
		size_t len;

		dest = elf_getProgramHeaderPaddr(elfFile, i);
		dbg_printf(DL_BASIC, "Elf file pheader physical: 0x%x\n", (unsigned int)dest);
		dbg_printf(DL_BASIC, "Elf file pheader virtual: 0x%x\n",
			       (unsigned int)elf_getProgramHeaderVaddr(elfFile,i));
		len = elf_getProgramHeaderFileSize(elfFile, i);
		dbg_printf(DL_BASIC, "This section's size in file: %p\n", len);
		src = (uint32_t) (uintptr_t) elfFile + elf_getProgramHeaderOffset(elfFile, i);
		dbg_printf(DL_BASIC, "Elf program header offset: %p\n", src);
		dbg_printf(DL_BASIC, "Copying to range from 0x%x to 0x%x of size: 0x%x\n", 
			(unsigned int)dest, (unsigned int)dest + (unsigned int)len, (unsigned int)len);
		memcpy((void*) (uintptr_t) dest, (void*) (uintptr_t) src, len);
		dest += len;
	}
	dbg_printf(DL_BASIC, "\n");

	return elf_getEntryPoint(elfFile);
}
