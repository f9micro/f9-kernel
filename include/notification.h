/* Copyright (c) 2026 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef NOTIFICATION_H_
#define NOTIFICATION_H_

#include <types.h>

/**
 * @file notification.h
 * @brief Unified Notification System (Event-Chaining) - CORE FEATURE
 *
 * F9's unified notification system provides three layers of event delivery.
 * All layers are ALWAYS available (non-configurable, following seL4 model).
 *
 * 1. BASIC NOTIFICATIONS:
 *    - Direct bit signaling for IPC (notification_signal)
 *    - Synchronous delivery (50-900 cycles)
 *    - Minimal overhead, atomic operations
 *
 * 2. ASYNCHRONOUS NOTIFICATIONS:
 *    - Queue-based async delivery (notification_post)
 *    - ASYNC_SOFTIRQ batched processing
 *    - RT-safe bounded processing (200 cycle WCET)
 *    - IRQ-safe posting from any context
 *    - Fast-path delivery (notification_post_softirq)
 *
 * 3. NOTIFICATION MASKS:
 *    - Multi-bit flag aggregation (32-bit masks)
 *    - OR/AND wait semantics
 *    - Multi-source event coordination
 *    - Up to 8 waiters per mask
 *
 * Design principles:
 * - L4 Constraint: Callbacks are kernel-internal only
 * - Minimal Overhead: Efficient event delivery
 * - RT-Safe: Bounded execution times
 * - Lock-Free: IRQ-safe operations
 * - Core Feature: Always enabled (aligned with seL4)
 */

/* Forward declarations */
struct tcb;
typedef struct tcb tcb_t;

/* Basic notifications (always available) */

#define NOTIFY_SIGNAL_MAX 32 /* 32-bit event mask */

/**
 * Notification callback signature.
 * Called after notification delivery, with interrupts ENABLED.
 *
 * @param tcb Thread that received notification (callback owner)
 * @param notify_bits Notification bit mask (which bits were signaled)
 * @param notify_data Optional event-specific data (32-bit payload)
 *
 * SAFETY: Must be internal kernel handler only. Do NOT expose to user-space.
 */
typedef void (*notify_handler_t)(tcb_t *tcb,
                                 uint32_t notify_bits,
                                 uint32_t notify_data);

/**
 * Register notification callback for thread.
 *
 * @param tcb Thread to receive notifications
 * @param handler Callback function (NULL to disable)
 */
void thread_set_ipc_notify_callback(tcb_t *tcb, notify_handler_t handler);

/**
 * Get current notification handler for thread.
 *
 * @param tcb Thread to query
 * @return Current handler or NULL if none
 */
notify_handler_t thread_get_ipc_notify_callback(tcb_t *tcb);

/**
 * Signal notification bits (direct delivery).
 * Used for synchronous IPC notification.
 *
 * @param tcb Target thread
 * @param bits Event bits to set (OR operation)
 */
void notification_signal(tcb_t *tcb, uint32_t bits);

/**
 * Clear notification bits.
 *
 * @param tcb Target thread
 * @param bits Event bits to clear (AND NOT operation)
 */
void notification_clear(tcb_t *tcb, uint32_t bits);

/**
 * Get current notification bits.
 *
 * @param tcb Target thread
 * @return Current event mask
 */
uint32_t notification_get(tcb_t *tcb);

/**
 * Atomic read-and-clear notification bits.
 * Prevents "lost wakeup" race condition.
 *
 * @param tcb Target thread
 * @param mask Bits to clear (0xFFFFFFFF for all)
 * @return Bits that were set before clearing
 */
uint32_t notification_read_clear(tcb_t *tcb, uint32_t mask);

/**
 * Wake thread blocked on SYS_NOTIFY_WAIT with proper semantics.
 *
 * Implements the full notification wake protocol:
 * 1. Check if thread is T_NOTIFY_BLOCKED (not T_RECV_BLOCKED)
 * 2. Check if signaled bits match thread's notify_mask
 * 3. Clear matched bits from notify_bits
 * 4. Write matched bits to thread's saved R0 (return value)
 * 5. Clear notify_mask and transition to T_RUNNABLE
 *
 * T_RECV_BLOCKED threads are NOT woken - they're waiting for IPC.
 *
 * @param thr Thread to potentially wake
 * @return 1 if thread was woken, 0 otherwise
 */
int notify_wake_thread(tcb_t *thr);

/**
 * Extended notification event structure.
 * Contains both notification bits and optional event data payload.
 */
typedef struct {
    uint32_t notify_bits; /* Notification bit mask that was signaled */
    uint32_t event_data;  /* Optional event-specific data (e.g., IRQ number for
                             high IRQs) */
} notification_event_t;

/**
 * Get current notification bits with extended data.
 * Retrieves both notification mask and event-specific payload.
 *
 * IMPORTANT: For high IRQs (≥31), the IRQ number is encoded in event_data.
 * - Low IRQs (0-30): notify_bits has bit at position (1 << irq)
 * - High IRQs (31+): notify_bits has bit 31 set, event_data contains actual IRQ
 * number
 * Note: IRQ 31 is treated as high IRQ to avoid ambiguity with bit 31 sentinel
 *
 * @param tcb Target thread
 * @param out_event Output structure to receive notification bits and data
 * @return 0 on success, -1 on error
 *
 * USAGE:
 *   notification_event_t event;
 *   notification_get_extended(current_thread, &event);
 *   if (event.notify_bits & (1 << 31)) {
 *       int irq_num = event.event_data;  // Decode high IRQ number
 *   }
 */
int notification_get_extended(tcb_t *tcb, notification_event_t *out_event);

/* Asynchronous notifications (queue-based delivery) */

/**
 * notification_post_softirq - Fast-path softirq-safe notification delivery
 *
 * Delivers notification immediately in softirq context, bypassing async queue.
 * Optimized fast path for timer notifications, eliminating queue allocation
 * and second softirq hop.
 *
 * @param thr Target thread to notify (must be valid TCB)
 * @param notify_bits Notification bit mask to signal (OR'ed with existing)
 *
 * @return 0 on success, -1 on error (not softirq context or invalid thread)
 *
 * CONTEXT: Softirq-only (not IRQ-safe, enforced by runtime check)
 * LATENCY: 250-450 cycles (3-4x faster than async queue)
 * JITTER: <100 cycles (5x better than async queue ±500 cycles)
 * SAFETY: Maintains softirq safety, no IRQ-context execution
 */
int notification_post_softirq(tcb_t *thr, uint32_t notify_bits);

/**
 * Post asynchronous notification to target thread.
 *
 * Events are queued and delivered asynchronously via ASYNC_SOFTIRQ.
 * If queue is full, event is silently dropped (best-effort delivery).
 *
 * @param thr Target thread to notify (must be valid TCB)
 * @param notify_bits Notification bit mask to signal (OR'ed with existing)
 * @param event_data Optional event-specific data (32-bit payload)
 *
 * @return 0 on success, -1 if queue full or invalid thread
 *
 * CONTEXT: IRQ-safe, can be called from any context
 * LATENCY: Async delivery via ASYNC_SOFTIRQ (150-200 cycles typical)
 * RT-SAFE: Bounded processing (CONFIG_ASYNC_EVENT_BOUNDED_PROCESSING)
 */
int notification_post(tcb_t *thr, uint32_t notify_bits, uint32_t event_data);

/**
 * Get number of pending async notifications in queue.
 *
 * @return Number of events currently queued for delivery
 *
 * NOTE: Snapshot value, may change immediately
 */
uint32_t notification_queue_depth(void);

/**
 * Check if async notification queue is full.
 *
 * @return 1 if queue full, 0 if space available
 */
int notification_queue_full(void);

/* Notification masks (multi-bit aggregation) */

/* Maximum waiters per notification mask */
#define NOTIFICATION_MASK_MAX_WAITERS 8

/* Wait options for notification masks */
#define NOTIFICATION_MASK_OR 0  /* Wait for ANY flag in mask */
#define NOTIFICATION_MASK_AND 1 /* Wait for ALL flags in mask */

/**
 * Notification mask structure.
 * Multi-bit notification object for event aggregation.
 */
typedef struct notification_mask {
    uint32_t id;            /* Mask ID (debug) */
    uint32_t current_flags; /* Current flag state */
    /* Waiting thread IDs (safe) */
    l4_thread_t waiter_ids[NOTIFICATION_MASK_MAX_WAITERS];
    uint32_t waiter_masks[NOTIFICATION_MASK_MAX_WAITERS]; /* Wait conditions */
    uint32_t notify_bits[NOTIFICATION_MASK_MAX_WAITERS]; /* Notification bits */
    uint8_t waiter_options[NOTIFICATION_MASK_MAX_WAITERS]; /* OR/AND options */
    uint8_t num_waiters;                                   /* Active waiters */
    uint8_t flags;                                         /* Reserved flags */
    const char *name;                                      /* Debug name */
} notification_mask_t;

/**
 * Create notification mask.
 *
 * @param mask Notification mask structure (user-provided)
 * @param name Optional debug name (can be NULL)
 * @return 0 on success, -1 on error
 */
int notification_mask_create(notification_mask_t *mask, const char *name);

/**
 * Delete notification mask.
 * Removes all waiters without notification.
 *
 * @param mask Notification mask to delete
 * @return 0 on success, -1 on error
 */
int notification_mask_delete(notification_mask_t *mask);

/**
 * Set flags in notification mask (OR with current flags).
 * Notifies any waiters whose conditions are now satisfied.
 *
 * @param mask Notification mask
 * @param flags_to_set Flags to set (OR'ed with current)
 * @return 0 on success, -1 on error
 *
 * IRQ-SAFE: Can be called from interrupt context
 * RT-SAFE: WCET ~170 cycles (8 waiters)
 */
int notification_mask_set(notification_mask_t *mask, uint32_t flags_to_set);

/**
 * Clear flags in notification mask (AND NOT with current flags).
 * Does not notify waiters (only set operations notify).
 *
 * @param mask Notification mask
 * @param flags_to_clear Flags to clear (AND NOT with current)
 * @return 0 on success, -1 on error
 *
 * IRQ-SAFE: Can be called from interrupt context
 * RT-SAFE: WCET ~50 cycles
 */
int notification_mask_clear(notification_mask_t *mask, uint32_t flags_to_clear);

/**
 * Get current flags (non-destructive read).
 *
 * @param mask Notification mask
 * @return Current flags value
 */
uint32_t notification_mask_get(notification_mask_t *mask);

/**
 * Register thread to wait for notification mask flags.
 * Thread receives notification when requested flags become available.
 *
 * @param mask Notification mask
 * @param requested_flags Flags to wait for (bit mask)
 * @param wait_option NOTIFICATION_MASK_OR or NOTIFICATION_MASK_AND
 * @param thread Thread to notify
 * @param notify_bit Notification bit for this event
 * @return 0 on success, -1 on error (mask full)
 *
 * NON-BLOCKING: Returns immediately, notification via callback
 * IMMEDIATE: If condition already met, notifies immediately
 */
int notification_mask_wait(notification_mask_t *mask,
                           uint32_t requested_flags,
                           uint8_t wait_option,
                           tcb_t *thread,
                           uint32_t notify_bit);

/**
 * Unregister thread from waiting on notification mask.
 * Removes thread from waiter list without notification.
 *
 * @param mask Notification mask
 * @param thread Thread to remove from waiters
 * @return 0 on success, -1 if not waiting
 */
int notification_mask_unwait(notification_mask_t *mask, tcb_t *thread);

/**
 * Get number of active waiters on notification mask.
 *
 * @param mask Notification mask
 * @return Number of active waiters (0-8)
 */
uint8_t notification_mask_waiter_count(notification_mask_t *mask);

#ifdef CONFIG_KDB
/**
 * KDB command: dump notification system statistics
 */
void kdb_dump_notifications(void);
#endif

#endif /* NOTIFICATION_H_ */
