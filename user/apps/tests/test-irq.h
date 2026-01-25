/* Copyright (c) 2026 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __TEST_IRQ_H__
#define __TEST_IRQ_H__

#include <platform/link.h>

#ifdef CONFIG_EXTI_INTERRUPT_TEST

/* IRQ test (test-irq.c) */
__USER_TEXT
void test_irq_exti(void);

#endif /* CONFIG_EXTI_INTERRUPT_TEST */

#endif /* __TEST_IRQ_H__ */
