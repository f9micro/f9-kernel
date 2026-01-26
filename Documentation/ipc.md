# Inter-Process Communication (IPC)

IPC is the central mechanism in F9, enabling synchronous message passing between threads and implementing functionality that would traditionally require additional system calls.

## Design Philosophy

L4 microkernels achieve high performance by minimizing the kernel and pushing functionality to user space. IPC serves as the universal interface:
- Threads communicate through messages, not shared state
- System services are implemented as user-space servers
- Hardware interrupts become IPC messages to handler threads
- Memory mapping operations are encoded in IPC message types

F9 follows this philosophy, providing only two system calls: ThreadControl and IPC.

## IPC System Call

The IPC system call handles all message passing:

```c
L4_MsgTag_t L4_Ipc(L4_ThreadId_t to,
                   L4_ThreadId_t FromSpecifier,
                   L4_Word_t Timeouts,
                   L4_ThreadId_t *from);
```

Parameters:
- `to`: Destination thread for sending (L4_NILTHREAD to receive only)
- `FromSpecifier`: Source filter for receiving (L4_NILTHREAD to send only)
- `Timeouts`: Combined send and receive timeout values
- `from`: Output parameter receiving the actual sender's ID

Combining send and receive in one call enables efficient request-response patterns.

## Message Structure

IPC messages consist of a header and a sequence of words:

```
+--------+----------+----------+-----+----------+
| Header | Untyped0 | Untyped1 | ... | Typed0   |
+--------+----------+----------+-----+----------+
```

### Message Header (MR0)

The first message register contains the tag:

```c
typedef union {
    struct {
        uint32_t n_untyped : 6;  /* Count of untyped words */
        uint32_t n_typed : 6;    /* Count of typed words */
        uint32_t flags : 4;      /* Message flags */
        uint32_t label : 16;     /* Application-defined label */
    } s;
    uint32_t raw;
} ipc_msg_tag_t;
```

### Untyped Words

Raw 32-bit values with application-defined meaning. The communicating threads agree on interpretation.

### Typed Words

Structured data that the kernel interprets, primarily for memory mapping:

- Map items: Share memory between address spaces
- Grant items: Transfer memory ownership

## Message Registers

F9 provides 16 message registers (MR0-MR15):
- MR0-MR7: Stored in the thread's context (`ctx.regs[0-7]`)
- MR8-MR15: Stored in the UTCB (`utcb->mr[0-7]`)

The first 8 MRs reuse the saved CPU registers in the context structure, avoiding separate storage. This design enables efficient IPC: message data is already in the context when the thread resumes.

```c
uint32_t ipc_read_mr(tcb_t *from, int i)
{
    if (i >= 8)
        return from->utcb->mr[i - 8];
    return from->ctx.regs[i];
}

void ipc_write_mr(tcb_t *to, int i, uint32_t data)
{
    if (i >= 8)
        to->utcb->mr[i - 8] = data;
    else
        to->ctx.regs[i] = data;
}
```

For small messages (up to 8 words), IPC completes without UTCB access, minimizing memory operations.

## IPC Operations

### Send Only

```c
L4_Ipc(destination, L4_NILTHREAD, timeout, NULL);
```

The caller blocks until the destination receives the message or timeout expires.

### Receive Only

```c
L4_Ipc(L4_NILTHREAD, source, timeout, &actual_sender);
```

The caller blocks until a message arrives from the specified source (or any thread if `L4_ANYTHREAD`).

### Send and Receive (Call)

```c
L4_Ipc(destination, destination, timeout, &actual_sender);
```

Send a message and wait for a reply from the same thread. This is the common RPC pattern.

## Special IPC Operations

F9 extends standard IPC with special thread IDs that trigger kernel operations:

### Thread Startup

Sending to a thread in `T_INACTIVE` state with the proper message format starts the thread:

```c
L4_Word_t msgs[5] = {
    (L4_Word_t)entry_point,
    (L4_Word_t)stack_pointer,
    (L4_Word_t)stack_size,
    0,  /* Reserved */
    0   /* Reserved */
};
```

### Memory Mapping

Typed words in IPC messages encode map/grant operations. The kernel intercepts these and performs the address space modifications.

### Logging

Sending to `THREAD_LOG` passes variable arguments to the kernel's logging system, allowing user threads to share a single UART without coordination.

### Interrupt Registration

Sending to `THREAD_IRQ_REQUEST` configures interrupt delegation (see interrupt.md).

## Kernel Implementation

### sys_ipc Function

The `sys_ipc()` function orchestrates IPC handling:
1. Check for special thread IDs (logging, interrupt config, etc.)
2. For normal IPC, locate the destination thread
3. If destination is ready to receive, perform immediate transfer
4. Otherwise, block the sender and update scheduler state

### do_ipc Function

The `do_ipc()` function performs the actual message transfer:
1. Read the message tag from the sender
2. Copy untyped words from sender's MRs to receiver's MRs
3. Process typed words (map/grant operations)
4. Update thread states for scheduling

```c
static void do_ipc(tcb_t *from, tcb_t *to)
{
    ipc_msg_tag_t tag = { .raw = ipc_read_mr(from, 0) };
    int untyped_last = tag.s.n_untyped + 1;
    int typed_last = untyped_last + tag.s.n_typed;

    /* Copy tag */
    ipc_write_mr(to, 0, tag.raw);

    /* Copy untyped words */
    for (int i = 1; i < untyped_last; ++i) {
        ipc_write_mr(to, i, ipc_read_mr(from, i));
    }

    /* Process typed words (map/grant) */
    for (int i = untyped_last; i < typed_last; ++i) {
        /* ... handle map items ... */
    }
}
```

## User-Space API

### Message Construction

```c
L4_Msg_t msg;
L4_MsgClear(&msg);

L4_Word_t data[3] = { value1, value2, value3 };
L4_MsgPut(&msg, label, 3, data, 0, NULL);
L4_MsgLoad(&msg);
```

### Sending Messages

```c
L4_MsgTag_t result = L4_Ipc(dest, L4_NILTHREAD, 0, NULL);
if (L4_IpcFailed(result)) {
    /* Handle error */
}
```

### Receiving Messages

```c
L4_ThreadId_t sender;
L4_MsgTag_t tag = L4_Ipc(L4_NILTHREAD, L4_ANYTHREAD, 0, &sender);

L4_Msg_t msg;
L4_MsgStore(tag, &msg);
L4_Word_t value = L4_MsgWord(&msg, 0);
```

## Performance Considerations

The L4 IPC design prioritizes performance:
- Direct process switch: The scheduler immediately runs the IPC partner
- Register-based MRs: Small messages avoid memory access
- Minimal copying: Only the specified words are transferred
- Combined operations: Send-receive in one system call

## Timeouts

IPC operations can specify timeouts to prevent indefinite blocking:
- Zero timeout: Non-blocking operation
- Infinite timeout: Block until completion
- Finite timeout: Block for specified duration, then abort

Timeouts are managed through the kernel timer event system (see [ktimer.md](ktimer.md)).
