K22 Programming Assignment 1

Q1. A process with a high nivcsw count has often been preempted involuntarily,
for example because it is CPU-bound and competes with other runnable tasks or
because a higher-priority task needed the CPU.

Q2. A process with a high nvcsw count often gives up the CPU while waiting,
such as an I/O-bound service waiting for input, a process waiting on a lock,
or a task that explicitly yields.

Q3. start_time is measured from boot using monotonic time, so processes
created during early startup can have very small values. This alone does not
identify a process as a user program or a kernel thread. The process name and
whether it has user-space state are better clues; for example, kernel threads
do not have a meaningful userpc.

Q4. A non-null userpc means the task has a saved user-mode instruction pointer
that can be reported as its return-to-user location. It does not necessarily
mean that the task was executing in user mode at the instant of the snapshot.

Q5. The process list and task state can change between the two calls. A
process shown in the first output but not the second may have exited (or no
longer matched the PC condition) before the next snapshot. The swapper/0
entry is the idle kernel task and is expected to remain; its userpc and
kernelpc are null because it has no saved user-mode context and no saved
schedule return address in the reported state. The sample alone cannot prove
why cupsd disappeared, but process lifetime and scheduling state make this
kind of difference possible.

Submission assessment:
Part 1 (90%): the patch implements the syscall, UAPI structure, process-tree
traversal, and syscall-number registration for x86-64 and the generic syscall
table used by ARM64. Part 2 (10%): the userspace program retries with larger
buffers, prints the returned process tree with indentation, and reports errors.
The userspace program builds with the supplied Makefile. Runtime validation of
the syscall still requires booting a kernel built with the patch, so this
assessment is based on implementation and build checks rather than a live
syscall run.

Build the userspace program:
  make -C userspace

If the header was installed into a kernel source tree's usr/include directory:
  make -C userspace KERNEL_HEADERS=/path/to/linux-7.2/usr/include

Run on the patched kernel:
  ./userspace/k22test
