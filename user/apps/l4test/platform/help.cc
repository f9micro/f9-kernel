/*********************************************************************
 *                
 * Copyright (C) 2003,  Karlsruhe University
 *                
 * File path:     l4test/ia32/help.cc
 * Description:   Helper finctions for ia32
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
 * $Id: help.cc,v 1.6 2006/10/21 01:59:54 reichelt Exp $
 *                
 ********************************************************************/
#include <l4/types.h>
#include <arch.h>

#include "../l4test.h"
#include "../assert.h"

void setup_exreg(L4_Word_t *ip, L4_Word_t *sp, void (*func)(void))

{
  L4_Word_t *stack;
  int max;

  /* work out the size in items */
  max = STACK_PAGES * PAGE_SIZE / sizeof( L4_Word_t );
  if (*sp == 0)
    {
      stack = (L4_Word_t*)get_pages( STACK_PAGES, 1 );
      assert( stack != NULL );
      *sp = (L4_Word_t)&stack[max-1];
    }

  *ip = (L4_Word_t) func;
}

void *code_addr(void *addr)

{
  return addr;
}

void
get_startup_values (void (*func)(void), L4_Word_t * ip, L4_Word_t * sp)
{
    // Calculate intial SP
    L4_Word_t stack = (L4_Word_t) get_pages (STACK_PAGES, 1);
    stack += STACK_PAGES * PAGE_SIZE;

    *ip = (L4_Word_t) func;
    *sp = stack;
}
