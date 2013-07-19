F9 microkernel
==============

This is `F9`, an experimental microkernel used to construct flexible embedded
systems inspired by famous L4 microkernel. The motivation of F9 microkernel
is to support running real-time and time-sharing application as well as
wireless communications for ARM Cortex-M series microprocessors concurrently.


Characteristics of F9 Microkernel
=================================
* F9 follows the fundamental principles of microkernels in that it implements
  address spaces, thread management, and IPC only in the privileged kernel.

* Regarding the interaction between a user thread and the microkernel, the
  concept of UTCB (user-level thread-control blocks) is being taken on. A UTCB
  is a small thread-specific region in the thread's virtual address space,
  which is always mapped. Therefore, the access to the UTCB can never raise a
  page fault, which makes it perfect for the kernel to access system-call
  arguments, in particular IPC payload copied from/to user threads.

* The kernel provides synchronous IPC (inter-process communication), for which
  short IPC carries payload in CPU registers only and full IPC copies message
  payload via the UTCBs of the communicating parties.

* Threads with the same priority are executed in a round-robin fashion


Licensing
==============

F9 microkernel is freely redistributable under the two-clause BSD License.
Use of this source code is governed by a BSD-style license that can be found
in the `LICENSE` file.
