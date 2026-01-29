/* Copyright (c) 2026 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <debug.h>
#include <init_hook.h>
#include <lib/ktable.h>
#include <notification.h>
#include <platform/irq.h>
#include <sched.h>
#include <softirq.h>
#include <thread.h>
#include <types.h>

/* Event-Chaining (Notification Objects) Implementation
 *
 * Simple callback storage without linked lists for minimal overhead.
 * Callbacks are invoked after IPC delivery, with interrupts enabled.
 *
 * ATOMICITY:
 * - On ARM Cortex-M, 32-bit aligned reads/writes are atomic.
 * - IRQ masking only needed for read-modify-write operations.
 * - Simple getters (thread_get_ipc_notify_callback, notification_get) do not
 * need IRQ masking.
 *
 * API CONSTRAINTS:
 * - All functions require non-NULL tcb parameter (checked at runtime).
 * - Callbacks must be kernel-internal handlers only (never user-space
 * pointers).
 * - Callbacks MUST NOT destroy their own TCB (use-after-free risk).
 * - All functions may be called from any context (IRQ-safe).
 */

/**
 * Update notify_pending flag based on current notify_bits.
 * Fast-path optimization: allows IPC path to skip notification checks
 * with single word read when no notifications pending.
 * MUST be called with IRQs disabled.
 */
static inline void update_notify_pending(tcb_t *tcb)
{
    tcb->notify_pending = (tcb->notify_bits != 0);
}

void thread_set_ipc_notify_callback(tcb_t *tcb, notify_handler_t handler)
{
    if (!tcb)
        return;

    /* Atomic storage - no IRQ masking needed on Cortex-M.
     * 32-bit aligned pointer write is atomic.
     */
    tcb->ipc_notify = handler;

    /* Data Memory Barrier: Ensure write completes before subsequent operations.
     * Important for multi-master systems (DMA) and prevents compiler
     * reordering.
     */
    __asm__ volatile("dmb" : : : "memory");
}

notify_handler_t thread_get_ipc_notify_callback(tcb_t *tcb)
{
    if (!tcb)
        return NULL;

    /* Atomic read - no IRQ masking needed on Cortex-M.
     * 32-bit aligned pointer read is atomic.
     */
    return tcb->ipc_notify;
}

void notification_signal(tcb_t *tcb, uint32_t bits)
{
    if (!tcb)
        return;

    /* Atomic OR operation for bit-mask notifications.
     * IRQ masking required for read-modify-write atomicity.
     */
    uint32_t flags = irq_save_flags();
    tcb->notify_bits |= bits;
    update_notify_pending(tcb);
    irq_restore_flags(flags);
}

void notification_clear(tcb_t *tcb, uint32_t bits)
{
    if (!tcb)
        return;

    /* Atomic AND NOT operation.
     * IRQ masking required for read-modify-write atomicity.
     */
    uint32_t flags = irq_save_flags();
    tcb->notify_bits &= ~bits;
    update_notify_pending(tcb);
    irq_restore_flags(flags);
}

uint32_t notification_get(tcb_t *tcb)
{
    if (!tcb)
        return 0;

    /* Atomic read - no IRQ masking needed on Cortex-M.
     * 32-bit aligned read is atomic.
     */
    return tcb->notify_bits;
}

int notification_get_extended(tcb_t *tcb, notification_event_t *out_event)
{
    if (!tcb || !out_event)
        return -1;

    /* Atomic read of both notify_bits and notify_data.
     * IRQ masking ensures consistency between the two fields.
     */
    uint32_t flags = irq_save_flags();
    out_event->notify_bits = tcb->notify_bits;
    out_event->event_data = tcb->notify_data;
    irq_restore_flags(flags);

    return 0;
}

uint32_t notification_read_clear(tcb_t *tcb, uint32_t mask)
{
    if (!tcb)
        return 0;

    /* Atomic read-modify-write to prevent "lost wakeup" race.
     * Scenario: Without atomicity:
     *   1. Thread reads bits (interrupts enabled)
     *   2. ISR signals new bit
     *   3. Thread clears old bits → new bit lost
     * Fix: Read and clear in single atomic operation.
     */
    uint32_t flags = irq_save_flags();
    uint32_t bits = tcb->notify_bits;
    tcb->notify_bits &= ~mask;
    update_notify_pending(tcb);
    irq_restore_flags(flags);

    return bits;
}

/* Asynchronous notifications (queue-based delivery) */

/* Async event structure
 * Uses thread ID (not raw pointer) for safe cross-reference - prevents
 * use-after-free if thread destroyed while event queued.
 */
typedef struct async_event {
    l4_thread_t
        target_id; /* Thread global ID (safe lookup via thread_by_globalid) */
    uint32_t notify_bits;     /* Notification bit mask */
    uint32_t event_data;      /* Optional 32-bit payload */
    struct async_event *next; /* Queue linkage */
} notification_async_t;

/* Event pool allocation */
DECLARE_KTABLE(notification_async_t,
               notification_async_table,
               CONFIG_MAX_NOTIFICATIONS);

/* Event queue (FIFO) */
static notification_async_t *notification_async_queue_head = NULL;
static notification_async_t *notification_async_queue_tail = NULL;

/* Queue depth counter for O(1) depth queries (prevents unbounded IRQ latency)
 */
static volatile uint32_t notification_async_queue_count = 0;

/* Statistics (debug/profiling) */
static uint32_t notification_async_posted = 0;
static uint32_t notification_async_delivered = 0;
static uint32_t notification_async_dropped = 0;

/* Number of softirq invocations */
static uint32_t notification_async_batches = 0;

/* Softirq reschedules (queue not empty) */
static uint32_t notification_async_reschedules = 0;

/* Fixed batch size for RT-safe bounded processing */
#define NOTIFICATION_BATCH_SIZE 4

/* Maximum pending events to display in KDB dump */
#define KDB_MAX_PENDING_DISPLAY 10

/**
 * Wake thread if blocked waiting for events.
 * Transitions T_RECV_BLOCKED -> T_RUNNABLE and enqueues for scheduling.
 */
static inline void wake_if_blocked(tcb_t *thr)
{
    if (thr->state == T_RECV_BLOCKED) {
        thr->state = T_RUNNABLE;
        sched_enqueue(thr);
    }
}

/**
 * notification_post_softirq - Direct softirq-safe notification delivery
 * @thr: target thread
 * @notify_bits: notification bits to signal
 *
 * Delivers notification immediately in softirq context, bypassing async queue.
 * This is the fast path for timer notifications, eliminating queue allocation
 * and the second softirq hop.
 *
 * SOFTIRQ-ONLY: Must be called from softirq context (not IRQ context).
 * FAST-PATH: ~100 cycles vs 150-1750 cycles for async queue path.
 * SAFE: Maintains softirq safety, no IRQ-context execution.
 * AGGREGATION: Uses notification_signal() for multi-bit OR semantics.
 *
 * Performance:
 *   - No queue allocation/deallocation overhead
 *   - No second softirq scheduling overhead
 *
 * Returns: 0 on success, -1 on error (not in softirq context or NULL thread)
 */
int notification_post_softirq(tcb_t *thr, uint32_t notify_bits)
{
    if (!thr)
        return -1;

    /* Safety check: must be in softirq context, not IRQ handler.
     * On ARM Cortex-M, IPSR=0 means thread/softirq mode, IPSR>0 means IRQ.
     * This prevents IRQ-context delivery which violates softirq safety.
     */
    if (irq_number() != 0) {
        dbg_printf(DL_NOTIFICATIONS,
                   "ERROR: notification_post_softirq called from IRQ context "
                   "(IPSR=%d)\n",
                   irq_number());
#ifdef CONFIG_DEBUG
        panic("notification_post_softirq: IRQ context violation (IPSR=%d)\n",
              irq_number());
#endif
        return -1;
    }

    /* Direct signal (atomic OR operation) */
    notification_signal(thr, notify_bits);

    wake_if_blocked(thr);

    dbg_printf(DL_NOTIFICATIONS,
               "SOFTIRQ: Fast-path delivery to %t bits=0x%x\n", thr->t_globalid,
               notify_bits);

    /* Update stats (reuse async counters for consistency) */
    notification_async_posted++;
    notification_async_delivered++;

    return 0;
}

/**
 * Post asynchronous event to target thread.
 *
 * IRQ-SAFE: Uses irq_save_flags/irq_restore_flags for atomicity.
 * QUEUE-FULL: Silently drops event if pool exhausted (best-effort).
 * SOFTIRQ: Schedules NOTIFICATION_SOFTIRQ for RT-safe bounded batch delivery.
 */
int notification_post(tcb_t *thr, uint32_t notify_bits, uint32_t event_data)
{
    notification_async_t *event;

    if (!thr)
        return -1;

    /* Queue event for softirq delivery (RT-safe bounded processing) */

    /* Allocate event from pool (may fail if queue full) */
    event = (notification_async_t *) ktable_alloc(&notification_async_table);
    if (!event) {
        /* Queue full - drop event (best-effort delivery) */
        notification_async_dropped++;

        dbg_printf(DL_NOTIFICATIONS,
                   "ASYNC: WARNING - Event dropped (queue full) for thread %t\n"
                   "  Consider increasing CONFIG_MAX_NOTIFICATIONS\n",
                   thr->t_globalid);

        return -1;
    }

    /* Initialize event */
    event->target_id = thr->t_globalid;
    event->notify_bits = notify_bits;
    event->event_data = event_data;
    event->next = NULL;

    /* Atomically enqueue event (IRQ-safe) */
    uint32_t flags = irq_save_flags();

    if (!notification_async_queue_head) {
        /* Queue was empty */
        notification_async_queue_head = notification_async_queue_tail = event;
    } else {
        /* Append to tail */
        notification_async_queue_tail->next = event;
        notification_async_queue_tail = event;
    }

    notification_async_queue_count++;
    notification_async_posted++;

    /* Schedule softirq for batch delivery */
    softirq_schedule(NOTIFICATION_SOFTIRQ);

    irq_restore_flags(flags);

    dbg_printf(DL_NOTIFICATIONS,
               "ASYNC: Posted event to %t bits=0x%x data=0x%x\n",
               thr->t_globalid, notify_bits, event_data);

    return 0;
}

/**
 * Softirq handler: process async event queue.
 *
 * CONTEXT: Softirq (interrupts enabled, preemption possible).
 * BATCH: Processes bounded number of events per invocation for RT-safety.
 * DELIVERY: Uses notification_signal() for Event-Chaining integration.
 * WAKEUP: Wakes blocked threads directly (scheduler optimization).
 * RT-SAFETY: Bounded processing ensures deterministic WCET.
 */
static void notification_async_handler(void)
{
    notification_async_t *event;
    uint32_t delivered = 0;

    /* RT-safe bounded processing: at most NOTIFICATION_BATCH_SIZE events
     * per invocation for predictable WCET. Remaining events trigger reschedule.
     */
    notification_async_batches++;

    while (delivered < NOTIFICATION_BATCH_SIZE) {
        uint32_t flags = irq_save_flags();

        /* Dequeue next event */
        event = notification_async_queue_head;
        if (!event) {
            /* Queue empty - done */
            irq_restore_flags(flags);
            break;
        }

        notification_async_queue_head = event->next;
        if (!notification_async_queue_head)
            notification_async_queue_tail = NULL;

        irq_restore_flags(flags);

        /* Deliver notification to target thread.
         * Event-Chaining callback will execute when thread next runs.
         * Lookup thread by ID to handle case where thread was destroyed
         * while event was queued (prevents use-after-free).
         */
        tcb_t *thr = thread_by_globalid(event->target_id);
        if (!thr) {
            /* Thread destroyed before delivery - drop event safely */
            dbg_printf(DL_NOTIFICATIONS,
                       "ASYNC: Dropping event for dead thread %t\n",
                       event->target_id);
            ktable_free(&notification_async_table, event);
            notification_async_queue_count--;
            continue;
        }

        dbg_printf(DL_NOTIFICATIONS,
                   "ASYNC: Delivering event to %t bits=0x%x data=0x%x\n",
                   thr->t_globalid, event->notify_bits, event->event_data);

        /* Signal notification bits (OR'ed with existing) and store event data
         */
        uint32_t signal_flags = irq_save_flags();
        thr->notify_bits |= event->notify_bits;
        thr->notify_data = event->event_data; /* Store most recent event_data */
        update_notify_pending(thr);
        irq_restore_flags(signal_flags);

        /* Wake thread if blocked waiting for events.
         * Callback (if set) executes after scheduler runs the thread.
         */
        wake_if_blocked(thr);

        /* Free event back to pool */
        ktable_free(&notification_async_table, event);
        notification_async_queue_count--;

        delivered++;
    }

    notification_async_delivered += delivered;

    if (delivered > 0) {
        dbg_printf(DL_NOTIFICATIONS,
                   "ASYNC: Batch delivered %d events (total: posted=%d "
                   "delivered=%d dropped=%d)\n",
                   delivered, notification_async_posted,
                   notification_async_delivered, notification_async_dropped);
    }

    /* If queue still has events, reschedule softirq for next batch.
     * This ensures incremental processing while maintaining bounded latency.
     */
    if (notification_async_queue_head) {
        softirq_schedule(NOTIFICATION_SOFTIRQ);
        notification_async_reschedules++;

        dbg_printf(DL_NOTIFICATIONS,
                   "ASYNC: Queue still has events, rescheduling softirq "
                   "(reschedules=%d)\n",
                   notification_async_reschedules);
    }
}

/**
 * Get number of pending async events in queue.
 * O(1) operation using maintained counter (prevents unbounded IRQ latency).
 * NOTE: Snapshot value, may change immediately.
 */
uint32_t notification_queue_depth(void)
{
    return notification_async_queue_count;
}

/**
 * Check if async event queue is full.
 * O(1) operation using maintained counter.
 * NOTE: This is a snapshot - queue may fill immediately after this check.
 */
int notification_queue_full(void)
{
    return (notification_async_queue_count >= CONFIG_MAX_NOTIFICATIONS);
}

/**
 * Initialize async event subsystem.
 * - Allocate event pool
 * - Register NOTIFICATION_SOFTIRQ handler
 */
static void notification_async_init(void)
{
    ktable_init(&notification_async_table);
    softirq_register(NOTIFICATION_SOFTIRQ, notification_async_handler);

    dbg_printf(DL_NOTIFICATIONS, "ASYNC: Initialized (pool size=%d)\n",
               CONFIG_MAX_NOTIFICATIONS);
}

/* Notification masks (multi-bit aggregation)
 *
 * SAFETY: Uses thread ID storage (not raw pointers) to prevent use-after-free.
 * If a thread is destroyed while waiting on a mask, notification_mask_set()
 * detects this via safe lookup (thread_by_globalid returns NULL) and
 * automatically clears the slot. This matches the IRQ system safe pattern.
 */

/* Global ID counter for debugging */
static uint32_t notification_mask_id_counter = 1;

/* Statistics (debug/profiling) */
static uint32_t notification_mask_created = 0;
static uint32_t notification_mask_deleted = 0;
static uint32_t notification_mask_sets = 0;
static uint32_t notification_mask_clears = 0;
static uint32_t notification_mask_waits = 0;
static uint32_t notification_mask_unwaits = 0;
static uint32_t notification_mask_notifications = 0;

/**
 * Check if waiter's condition is satisfied by current flags.
 *
 * @param current_flags  Current flag state
 * @param waiter_mask    Flags the waiter is waiting for
 * @param wait_option    NOTIFICATION_MASK_OR or NOTIFICATION_MASK_AND
 * @return               1 if condition met, 0 otherwise
 */
static inline int waiter_condition_met(uint32_t current_flags,
                                       uint32_t waiter_mask,
                                       uint8_t wait_option)
{
    if (wait_option == NOTIFICATION_MASK_OR)
        return (current_flags & waiter_mask) != 0;

    /* AND: All flags in mask must be set */
    return (current_flags & waiter_mask) == waiter_mask;
}

/**
 * Clear all waiter slots in a notification mask.
 * Uses L4_NILTHREAD (not NULL) for empty thread ID slots.
 * MUST be called with IRQs disabled.
 */
static void clear_waiter_slots(notification_mask_t *group)
{
    group->num_waiters = 0;
    for (int i = 0; i < NOTIFICATION_MASK_MAX_WAITERS; i++) {
        group->waiter_ids[i] = L4_NILTHREAD;
        group->waiter_masks[i] = 0;
        group->notify_bits[i] = 0;
        group->waiter_options[i] = 0;
    }
}

/**
 * Create notification mask (event flags group).
 * Initializes all waiter slots to L4_NILTHREAD for safe thread ID storage.
 * IRQ-safe.
 */
int notification_mask_create(notification_mask_t *group, const char *name)
{
    if (!group)
        return -1;

    uint32_t flags = irq_save_flags();

    group->id = notification_mask_id_counter++;
    group->current_flags = 0;
    group->flags = 0;
    group->name = name;
    clear_waiter_slots(group);

    notification_mask_created++;

    irq_restore_flags(flags);

    dbg_printf(DL_NOTIFICATIONS, "EVENT_FLAGS: Created group %d (%s)\n",
               group->id, name ? name : "unnamed");

    return 0;
}

/**
 * Delete notification mask (event flags group).
 * Removes all waiters without notification. Safe to call even if threads
 * are waiting (slots cleared with L4_NILTHREAD).
 * IRQ-safe.
 */
int notification_mask_delete(notification_mask_t *group)
{
    if (!group)
        return -1;

    uint32_t flags = irq_save_flags();

    dbg_printf(DL_NOTIFICATIONS, "EVENT_FLAGS: Deleting group %d (%s)\n",
               group->id, group->name ? group->name : "unnamed");

    clear_waiter_slots(group);
    group->current_flags = 0;
    group->id = 0;

    notification_mask_deleted++;

    irq_restore_flags(flags);

    return 0;
}

/**
 * Set event flags (OR with current flags).
 * Notifies any waiters whose conditions are now satisfied.
 *
 * SAFETY: Uses safe thread lookup (thread_by_globalid) with NULL check.
 * Automatically clears slots for destroyed threads. Prevents use-after-free.
 * IRQ-safe.
 */
int notification_mask_set(notification_mask_t *group, uint32_t flags_to_set)
{
    if (!group)
        return -1;

    uint32_t flags = irq_save_flags();

    uint32_t old_flags = group->current_flags;
    group->current_flags |= flags_to_set;

    notification_mask_sets++;

    dbg_printf(DL_NOTIFICATIONS,
               "EVENT_FLAGS: Set flags in group %d: 0x%x | 0x%x = 0x%x\n",
               group->id, old_flags, flags_to_set, group->current_flags);

    /* Check all waiters for newly satisfied conditions.
     * Safe lookup pattern: thread_by_globalid(id) with NULL check.
     * Automatically cleans up slots for destroyed threads.
     */
    for (int i = 0; i < NOTIFICATION_MASK_MAX_WAITERS; i++) {
        /* Skip empty slots */
        if (group->waiter_ids[i] == L4_NILTHREAD)
            continue;

        /* Safe thread lookup - handle thread destruction */
        tcb_t *waiter = thread_by_globalid(group->waiter_ids[i]);
        if (!waiter) {
            /* Thread destroyed - clear slot safely */
            dbg_printf(
                DL_NOTIFICATIONS,
                "EVENT_FLAGS: Waiter thread %t destroyed, clearing slot %d\n",
                group->waiter_ids[i], i);
            group->waiter_ids[i] = L4_NILTHREAD;
            group->waiter_masks[i] = 0;
            group->notify_bits[i] = 0;
            group->waiter_options[i] = 0;
            group->num_waiters--;
            continue;
        }

        if (!waiter_condition_met(group->current_flags, group->waiter_masks[i],
                                  group->waiter_options[i]))
            continue;

        dbg_printf(
            DL_NOTIFICATIONS,
            "EVENT_FLAGS: Notifying waiter %t (mask=0x%x, opt=%s)\n",
            waiter->t_globalid, group->waiter_masks[i],
            group->waiter_options[i] == NOTIFICATION_MASK_OR ? "OR" : "AND");

        notification_post(waiter, group->notify_bits[i], group->current_flags);

        notification_mask_notifications++;
    }

    irq_restore_flags(flags);

    return 0;
}

/**
 * Clear event flags (AND NOT with current flags).
 * Does not notify waiters.
 */
int notification_mask_clear(notification_mask_t *group, uint32_t flags_to_clear)
{
    if (!group)
        return -1;

    uint32_t flags = irq_save_flags();

    uint32_t old_flags = group->current_flags;
    group->current_flags &= ~flags_to_clear;

    notification_mask_clears++;

    dbg_printf(DL_NOTIFICATIONS,
               "EVENT_FLAGS: Clear flags in group %d: 0x%x & ~0x%x = 0x%x\n",
               group->id, old_flags, flags_to_clear, group->current_flags);

    irq_restore_flags(flags);

    return 0;
}

/**
 * Get current flags (non-destructive read).
 */
uint32_t notification_mask_get(notification_mask_t *group)
{
    if (!group)
        return 0;

    uint32_t flags = irq_save_flags();
    uint32_t current = group->current_flags;
    irq_restore_flags(flags);

    return current;
}

/**
 * Register thread to wait for notification mask flags.
 * Stores thread ID (not pointer) for safe cross-reference.
 *
 * @param group          Notification mask
 * @param requested_flags Flags to wait for (bit mask)
 * @param wait_option    NOTIFICATION_MASK_OR or NOTIFICATION_MASK_AND
 * @param thread         Thread to notify (stored as thread ID internally)
 * @param notify_bit     Notification bit for this event
 * @return               0 on success, -1 on error (mask full)
 *
 * IMMEDIATE: If condition already met, notifies immediately.
 * IRQ-safe.
 */
int notification_mask_wait(notification_mask_t *group,
                           uint32_t requested_flags,
                           uint8_t wait_option,
                           tcb_t *thread,
                           uint32_t notify_bit)
{
    if (!group || !thread)
        return -1;

    if (wait_option != NOTIFICATION_MASK_OR &&
        wait_option != NOTIFICATION_MASK_AND)
        return -1;

    uint32_t flags = irq_save_flags();

    /* Find slot: existing waiter (by thread ID) or first free slot.
     * Compares thread IDs (not pointers) for safe identification.
     */
    int slot = -1;
    int is_update = 0;

    for (int i = 0; i < NOTIFICATION_MASK_MAX_WAITERS; i++) {
        if (group->waiter_ids[i] == thread->t_globalid) {
            slot = i;
            is_update = 1;
            break;
        }
        if (slot < 0 && group->waiter_ids[i] == L4_NILTHREAD)
            slot = i;
    }

    if (slot < 0) {
        irq_restore_flags(flags);
        dbg_printf(DL_NOTIFICATIONS,
                   "EVENT_FLAGS: Failed to add waiter - group %d full\n",
                   group->id);
        return -1;
    }

    /* Update waiter slot.
     * Store thread ID (not pointer) for safe cross-reference.
     */
    group->waiter_ids[slot] = thread->t_globalid;
    group->waiter_masks[slot] = requested_flags;
    group->waiter_options[slot] = wait_option;
    group->notify_bits[slot] = notify_bit;

    if (!is_update) {
        group->num_waiters++;
        notification_mask_waits++;
    }

    dbg_printf(DL_NOTIFICATIONS,
               "EVENT_FLAGS: %s wait for thread %t in group %d "
               "(mask=0x%x, opt=%s, bit=0x%x)\n",
               is_update ? "Updated" : "Added", thread->t_globalid, group->id,
               requested_flags,
               wait_option == NOTIFICATION_MASK_OR ? "OR" : "AND", notify_bit);

    /* Check if condition is already met */
    if (waiter_condition_met(group->current_flags, requested_flags,
                             wait_option)) {
        dbg_printf(
            DL_NOTIFICATIONS,
            "EVENT_FLAGS: Condition already met, notifying immediately\n");
        notification_post(thread, notify_bit, group->current_flags);
        notification_mask_notifications++;
    }

    irq_restore_flags(flags);
    return 0;
}

/**
 * Unregister thread from waiting on notification mask.
 * Compares thread IDs (not pointers) for safe removal.
 *
 * @param group  Notification mask
 * @param thread Thread to remove from waiters
 * @return       0 on success, -1 if not waiting
 *
 * IRQ-safe.
 */
int notification_mask_unwait(notification_mask_t *group, tcb_t *thread)
{
    if (!group || !thread)
        return -1;

    uint32_t flags = irq_save_flags();

    /* Find and remove waiter.
     * Compares thread IDs (not pointers) for safe identification.
     */
    for (int i = 0; i < NOTIFICATION_MASK_MAX_WAITERS; i++) {
        if (group->waiter_ids[i] != thread->t_globalid)
            continue;

        group->waiter_ids[i] = L4_NILTHREAD;
        group->waiter_masks[i] = 0;
        group->waiter_options[i] = 0;
        group->notify_bits[i] = 0;
        group->num_waiters--;

        notification_mask_unwaits++;

        dbg_printf(DL_NOTIFICATIONS,
                   "EVENT_FLAGS: Removed thread %t from group %d\n",
                   thread->t_globalid, group->id);

        irq_restore_flags(flags);
        return 0;
    }

    irq_restore_flags(flags);
    return -1;
}

/**
 * Get number of active waiters.
 */
uint8_t notification_mask_waiter_count(notification_mask_t *group)
{
    if (!group)
        return 0;

    uint32_t flags = irq_save_flags();
    uint8_t count = group->num_waiters;
    irq_restore_flags(flags);

    return count;
}

#ifdef CONFIG_KDB
/**
 * KDB command: dump unified notification system statistics
 * Shows both async notifications and notification mask groups
 */
void kdb_dump_notifications(void)
{
    uint32_t depth = notification_queue_depth();

    dbg_printf(DL_KDB, "Async Notification Statistics:\n");
    dbg_printf(DL_KDB, "  Posted:    %d\n", notification_async_posted);
    dbg_printf(DL_KDB, "  Delivered: %d\n", notification_async_delivered);
    dbg_printf(DL_KDB, "  Dropped:   %d\n", notification_async_dropped);
    dbg_printf(DL_KDB, "  Pending:   %d\n", depth);
    dbg_printf(DL_KDB, "  Pool size: %d\n", CONFIG_MAX_NOTIFICATIONS);
    dbg_printf(DL_KDB, "  Pool free: %d\n", CONFIG_MAX_NOTIFICATIONS - depth);

    dbg_printf(DL_KDB, "\nBounded Processing (RT-safe):\n");
    dbg_printf(DL_KDB, "  Max batch size:   %d events\n",
               NOTIFICATION_BATCH_SIZE);
    dbg_printf(DL_KDB, "  Softirq calls:    %d\n", notification_async_batches);
    dbg_printf(DL_KDB, "  Reschedules:      %d\n",
               notification_async_reschedules);
    if (notification_async_batches > 0) {
        dbg_printf(DL_KDB, "  Avg events/batch: %d\n",
                   notification_async_delivered / notification_async_batches);
    }

    if (depth > 0) {
        dbg_printf(DL_KDB, "\nPending notifications:\n");

        uint32_t flags = irq_save_flags();
        notification_async_t *event = notification_async_queue_head;
        int idx = 0;

        while (event && idx < KDB_MAX_PENDING_DISPLAY) {
            dbg_printf(DL_KDB, "  [%d] target=%t bits=0x%x data=0x%x\n", idx,
                       event->target_id, event->notify_bits, event->event_data);
            event = event->next;
            idx++;
        }

        if (event)
            dbg_printf(DL_KDB, "  ... %d more\n", depth - idx);

        irq_restore_flags(flags);
    }

    dbg_printf(DL_KDB, "\nNotification Mask Statistics:\n");
    dbg_printf(DL_KDB, "  Created:       %d\n", notification_mask_created);
    dbg_printf(DL_KDB, "  Deleted:       %d\n", notification_mask_deleted);
    dbg_printf(DL_KDB, "  Active:        %d\n",
               notification_mask_created - notification_mask_deleted);
    dbg_printf(DL_KDB, "  Set ops:       %d\n", notification_mask_sets);
    dbg_printf(DL_KDB, "  Clear ops:     %d\n", notification_mask_clears);
    dbg_printf(DL_KDB, "  Wait regs:     %d\n", notification_mask_waits);
    dbg_printf(DL_KDB, "  Wait unregs:   %d\n", notification_mask_unwaits);
    dbg_printf(DL_KDB, "  Notifications: %d\n",
               notification_mask_notifications);
}
#endif /* CONFIG_KDB */

/* Initialize unified notification system at kernel startup */
INIT_HOOK(notification_async_init, INIT_LEVEL_KERNEL);
