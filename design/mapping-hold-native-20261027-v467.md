# Proposal B v467: PDP-6 VM mapping control-word test

The v466 RENAME gateway test uses a synthetic mapping-hold callback.
New testkit v467 instead links and runs the **actual** PDP-6
`vm_user_mapping_hold` and `vm_user_mapping_release` implementations.
It verifies that the u-area word 024 hold bit (02) is set and cleared
without clobbering unrelated control bits, including its left half;
the preexisting-bit and boot/no-u-area cases also PASS natively.

Scheduler source inspection establishes that `proc_wait_event` marks
the current process SLEEP, removes it from the run queue and requests
PI6 via CONO. A synchronous test callback cannot model actual PI6
context switching, suspended kernel stacks or interprocess wakeup.
An integrated scheduler-level test is still mandatory before removing
the syscall/FILE RENAME ownership boundary.

No production edit; accepted full linked lowmem KCORE 8962,
KCORE+MRES 15509 and PERMANENT_LAST 15556. 32K acceptance deferred.
