# Proposal B v470: successful native RENAME under multiprocess PDP-6

The daimos-testkit v470 full-system PDP-6 scheduler regression extends
v469 with a real successful writable physical-filesystem RENAME. Slot 5
creates a private `/CONFIG/R470A` D6FS file, writes a distinctive 36-bit
word, closes, renames it to `/CONFIG/R470B`, verifies that reopening the
old name fails, checks the word through the new name and unlinks the
result. The test also checks a failed RENAME on absent `/SYSTEM` names,
then performs disk I/O while the other processes run.

The full boot-image/SIMH regression PASS covers successful physical
RENAME, failure handling, multiprocess scheduling/preemption and disk
sleep/wakeup. Although this is genuine monitor-UUO FILE/VFS/provider
execution, the test does not instrument a PI6 context switch *inside*
the RENAME provider transaction; do not infer such a switch occurred.

No production source edit or RAM increase. Accepted GCC PDP-6 lowmem
KCORE 8962, KCORE+MRES 15509, PERMANENT_LAST 15556. The strict 32K
acceptance milestone remains deferred.
