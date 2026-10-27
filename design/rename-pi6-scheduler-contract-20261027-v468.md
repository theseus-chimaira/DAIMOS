# Proposal B v468: PI6 scheduling and RENAME ownership contract

The actual `proc_wait_event` moves a process from the run queue to
SLEEP, marks the event channel, and requests a software PI6 interrupt.
`proc_sched_pi_resched` saves a sleeping executive continuation with
`proc_save_kernel` and selects a runnable process. Restoration uses
the private saved kernel stack and syscall globals. The VM mapping
hold bit is stored in the process u-area, not in volatile registers.

New daimos-testkit static contract test v468 checks the RENAME syscall
mapping wrapper, hold/release bit operations, PI6 sleeping-kernel
save/restore path, and physical-provider wait/unlock machinery.
The static test PASS is an ABI/source guard, not behavioral proof.

The existing complete PDP-6 native multiprocess scheduler regression
`daimos-scheduler-multiproc-v1.sh` was rerun against this checkout and
PASS (preemption, process context, nice ABI, sleep/wakeup). It
exercises real scheduling and disk wait, but not RENAME during sleep.

The full integrated RENAME + actual PI6 cross-process switch with a
mapping lease remains unverified. No production edit; accepted linked
lowmem KCORE 8962, KCORE+MRES 15509, PERMANENT_LAST 15556.
Strict 32K acceptance deferred.
