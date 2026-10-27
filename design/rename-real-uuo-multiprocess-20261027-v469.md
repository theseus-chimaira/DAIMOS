# Proposal B v469: real monitor-UUO RENAME under multiprocess scheduling

New `daimos-testkit` full-system PDP-6 regression v469 derives from
the established native multiprocess scheduler test, preserving five
user processes, preemption, private-context assertions and disk-read
sleep/wakeup coverage. In process slot 5 the test now invokes the
actual `dsys_rename` monitor UUO on two deliberately nonexistent
names under `/SYSTEM`. It demands error -1, emits a distinct R
checkpoint, and then performs the normal disk sleep/wakeup probe.

Full native PDP-6 SIMH run PASS: the RENAME error path returns,
the process continues into disk I/O, and existing scheduler markers
and invariants hold. The operation is deliberately non-mutating;
the test does **not** prove successful RENAME across real physical
provider wait/resumption or the proposed syscall/FILE cutover.

No production source change or new permanent allocation. Accepted
linked GCC lowmem KCORE **8962**, KCORE+MRES 15509, PERMANENT_LAST
15556. Strict 32K acceptance stays deferred.
