/* Copyright (c) 2026 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/*
 * Fault Tests
 *
 * These tests trigger kernel faults to verify protection mechanisms.
 * Selected via FAULT_TYPE define passed from make (FAULT=mpu or FAULT=canary).
 *
 * FAULT_TYPE=1 (mpu): Write to code memory triggers MPU fault
 * FAULT_TYPE=2 (canary): Stack overflow corrupts canary, triggers panic
 */

#include <user_runtime.h>
#include <l4/thread.h>
#include <l4io.h>
#include "tests.h"

/* Marker for expected fault - parsed by qemu-test.py */
#define FAULT_EXPECT(name) \
	printf("[FAULT:EXPECT] %s\n", name)

#if FAULT_TYPE == FAULT_MPU
/*
 * MPU Fault Test
 *
 * Triggers an MPU fault by attempting to write to code memory.
 * The kernel should detect the violation and panic with "Memory fault".
 */

/*
 * This function's address will be used as the write target.
 * We attempt to write to this location to trigger MPU fault.
 */
__USER_TEXT
static void dummy_code_function(void)
{
	/* This function exists only to have a code address to write to */
	__asm__ volatile ("nop");
}

/*
 * Attempt to write to code memory.
 * This should trigger an MPU fault since code is execute-only or read-only.
 */
__USER_TEXT
static void trigger_mpu_fault(void)
{
	volatile uint32_t *code_ptr;

	/* Get address of code function */
	code_ptr = (volatile uint32_t *)(uintptr_t)dummy_code_function;

	printf("Attempting write to code at %p...\n", (void *)code_ptr);

	/* This write should trigger MPU DACCVIOL (data access violation) */
	*code_ptr = 0xDEADC0DE;

	/* If we get here, MPU is not protecting code - test fails */
	printf("[FAULT:UNEXPECTED] No fault occurred - MPU not protecting code!\n");
}

__USER_TEXT
void run_fault_test(void)
{
	printf("\n=== MPU Fault Test ===\n");

	/* Signal expected fault type */
	FAULT_EXPECT("mpu_write_to_code");

	/* Small delay to ensure output is flushed */
	L4_Sleep(L4_TimePeriod(10000));

	/* Trigger the fault - should not return */
	trigger_mpu_fault();

	/* Should never reach here */
	printf("[FAULT:ERROR] Test did not trigger expected fault\n");
}

#elif FAULT_TYPE == FAULT_CANARY
/*
 * Stack Canary Trip Test
 *
 * Triggers a stack overflow to corrupt the canary value.
 * The kernel should detect the corruption during context switch and panic.
 */

/* Counter to track recursion depth */
__USER_BSS static volatile int recursion_depth;

/*
 * Recursive function to consume stack space.
 * Uses volatile local array to prevent compiler optimization.
 * Each call consumes ~128 bytes of stack.
 *
 * The infinite recursion is intentional - we want to overflow the stack.
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winfinite-recursion"
__USER_TEXT
__attribute__((noinline))
static void stack_consumer(void)
{
	/* Local array to consume stack space */
	volatile char buffer[64];
	volatile char sum = 0;
	int i;

	/* Touch the buffer to prevent optimization */
	for (i = 0; i < 64; i++) {
		buffer[i] = (char)recursion_depth;
		sum += buffer[i];  /* Read buffer to satisfy compiler */
	}
	(void)sum;  /* Prevent unused warning */

	recursion_depth++;

	/* Print progress every 10 levels */
	if ((recursion_depth % 10) == 0) {
		printf("Recursion depth: %d\n", recursion_depth);
	}

	/* Recurse deeper - this will eventually overflow */
	stack_consumer();

	/* Should never reach here due to stack overflow */
	printf("Returned from depth %d\n", recursion_depth);
}
#pragma GCC diagnostic pop

/*
 * Directly corrupt the canary.
 * Search the entire stack region for the canary value.
 * Stack size is 2048 bytes (defined in main.c DECLARE_USER).
 */
#define STACK_SIZE_WORDS (2048 / sizeof(uint32_t))

__USER_TEXT
static void corrupt_canary_directly(void)
{
	uint32_t *stack_ptr;
	uint32_t *search_ptr;
	uint32_t *stack_limit;
	int found = 0;

	/* Get current stack pointer */
	__asm__ volatile ("mov %0, sp" : "=r" (stack_ptr));

	printf("Current SP: %p\n", (void *)stack_ptr);

	/* ARM stacks grow downward. Canary is at stack_base (lowest address).
	 * Search from current SP down to approximate stack limit.
	 * Be conservative to avoid faulting on invalid memory.
	 */
	stack_limit = stack_ptr - STACK_SIZE_WORDS;

	for (search_ptr = stack_limit; search_ptr < stack_ptr; search_ptr++) {
		if (*search_ptr == 0xDEADBEEF) {
			printf("Found canary at %p\n", (void *)search_ptr);
			printf("Corrupting canary...\n");
			*search_ptr = 0xBADCAFE;
			found = 1;
			break;
		}
	}

	if (!found) {
		printf("Warning: Canary not found in search range\n");
		printf("Trying recursion-based overflow...\n");
		stack_consumer();
	}

	/* Trigger context switch to detect corruption */
	printf("Triggering context switch...\n");
	L4_Sleep(L4_TimePeriod(1000));

	/* Should not reach here - kernel should panic */
	printf("[FAULT:UNEXPECTED] No panic after canary corruption!\n");
}

__USER_TEXT
void run_fault_test(void)
{
	printf("\n=== Stack Canary Trip Test ===\n");

	/* Signal expected fault type */
	FAULT_EXPECT("stack_canary_trip");

	/* Small delay to ensure output is flushed */
	L4_Sleep(L4_TimePeriod(10000));

	/* Initialize recursion counter */
	recursion_depth = 0;

	/* Try direct canary corruption first, fall back to recursion */
	corrupt_canary_directly();

	/* Should never reach here */
	printf("[FAULT:ERROR] Test did not trigger expected fault\n");
}

#else
/* No fault type selected - provide stub to avoid linker error */
__USER_TEXT
void run_fault_test(void)
{
	printf("[FAULT:ERROR] No fault type selected\n");
	printf("Use: make run-tests FAULT=mpu|canary\n");
}
#endif
