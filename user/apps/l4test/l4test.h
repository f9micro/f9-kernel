/*********************************************************************
 *                
 * Copyright (C) 1999-2010,  Karlsruhe University
 * Copyright (C) 2008-2009,  Volkmar Uhlig, IBM Corporation
 *                
 * File path:     l4test/l4test.h
 * Description:   
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
 * $Id$
 *                
 ********************************************************************/
#ifndef __L4TEST_H__
#define __L4TEST_H__

/* define this for ANSI menus */
//#define USE_ANSI 1
#include <stdbool.h>
#include "string.h"
#include <l4/kip.h>

#define ESC "\e["
#define LIGHT_RED    ESC "31;1m"
#define LIGHT_GREEN  ESC "32;1m"
#define LIGHT_BLUE   ESC "34;1m"
#define BLACK        ESC "0m"
#define HOME         ESC "0;0H"
#define CLEAR_LINE   ESC "K"
#define CLEAR_SCREEN ESC "2J"

#define STR_OK		(LIGHT_GREEN "OK" BLACK)
#define STR_FAILED	(LIGHT_RED "FAILED" BLACK)

/* memory stuff */
#define PAGE_MASK   (~(PAGE_SIZE-1))

#if !defined(NULL)
#define NULL	0
#endif

/* ex-reg flags */
#define EX_HALT (1<<0)
#define EX_RECV (1<<1)
#define EX_SEND (1<<2)
#define EX_SP   (1<<3)
#define EX_IP   (1<<4)
#define EX_FLAG (1<<5)
#define EX_USER (1<<6)
#define EX_PAGR (1<<7)

/* IPC flags */
#define IF_PHS (1<<0)  /* phase bit (same as below) */
#define IF_RCV (1<<0)  /* bit set if in recv phase */
#define IF_RDR (1<<1)  /* redirected bit */
#define IF_XPC (1<<2)  /* cross processor bit */
#define IF_ERR (1<<3)  /* error bit */

#define IPC_ERROR(tag) ((tag.X.flags & IF_ERR) == IF_ERR)

const char * ipc_errorcode (L4_Word_t errcode);
const char * ipc_errorphase (L4_Word_t errcode);


/* testing macros */
#define SET_MSG( msg, tst, ok, err) (msg = ((tst) ? ok : err))

/* globals */
extern void *next_touch;


/* boring output functions */
void print_h1( const char *msg );
void print_h2( const char *msg );
void print_result (const char * str, bool test);

/* good for lazy debugging :) */
void msec_sleep( L4_Word_t msec );

/* resource allocation */
void * get_pages( L4_Word_t count, int touch );
void *get_new_page(void);


/* do useful stuff */
L4_Word_t safe_mem_touch( void *addr );
void start_thread_ip_sp( L4_ThreadId_t tid, L4_Word_t ip, L4_Word_t sp );

/* Thread/address space management */
L4_ThreadId_t get_new_tid (void);
L4_ThreadId_t create_thread (void (*func)(void), bool new_space,
			     int cpu, L4_Word_t spacectrl);
L4_Word_t kill_thread (L4_ThreadId_t tid);
void start_thread (L4_ThreadId_t tid, void (*func)(void));


/* architecture helper functions */
void get_startup_values (void (*func)(void), L4_Word_t * ip, L4_Word_t * sp,
		L4_Word_t * stack_size);
void *code_addr( void *addr );
void setup_exreg( L4_Word_t *ip, L4_Word_t *sp, void (*func)(void) );


L4_INLINE bool l4_has_feature( const char *feature_name )
{
    void *kip = L4_GetKernelInterface();
    char *name;

    for( L4_Word_t i = 0; (name = L4_Feature(kip,i)) != '\0'; i++ )
	if( !strcmp(feature_name, name) )
	    return true;
    return false;
}




#endif /* !__L4TEST_H__ */
