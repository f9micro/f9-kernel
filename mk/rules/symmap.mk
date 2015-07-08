# Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


ifeq "$(CONFIG_SYMMAP)" "y"


cmd_elf_to_symmap = $(NM) $< | sort | cut -d' ' -f1,3 | \
	sed -n "H;/kernel_text_start/h;/end_of_text/{x;p;}"| \
	awk 'BEGIN { STRCOUNT = 0; COUNT = 0; } \
	{ \
		SYM = SYM "{ (void*) (0x"$$1"), "STRCOUNT" },\n"; \
		STRCOUNT += length($$2) + 1;\
		STRNAME = STRNAME "\"" $$2 "\\0" "\"" "\n"; \
		COUNT++; \
	} \
	END { \
		print "typedef struct _ksym { void *addr; int strid; } ksym;"; \
		print "\#define __SYMTAB  __attribute__ ((section(\".sym_tab\")))"; \
		print "int magic __SYMTAB = 0xA2CB;"; \
		print "int symcount __SYMTAB = " COUNT ";"; \
		print "ksym data[] __SYMTAB = {"; \
		printf "%s", SYM; \
		print "};"; \
		print "char strings [] __SYMTAB = {" ; \
		printf "%s", STRNAME; \
		print "};" \
	}' > $(out)/$*_symmap.c && \
	$(CC) $(CFLAGS) -c $(out)/$*_symmap.c -o $(out)/$*.symmap.o

$(out)/%.symmap.o: $(out)/f9_nosym.elf
	$(call quiet,elf_to_symmap,NM     )

symmap_obj-list-$(CONFIG_SYMMAP) += $(out)/$(PROJECT).symmap.o

endif
