# P1-C v1592: genuine two-owner PI6 wakeup with separate failed swap rollback

Date: 2026-11-15. Two full real 128K KCC PDP-6 cold boots using
four original DR0..DR3 Type-236 D6FS root VOLUME members and the
installed BLOCKSET interleave-mapped BACKSTORE, persistent MEMFS/XIP
and LOGSTORE. Production code, machine-visible RAM, ABI and media
format unchanged; the experiment changes only a disposable guest.

The accepted v1584 source observer marks synchronous MEMFS snapshot
entry, actual independent scheduler slot-0 swap transaction and
genuine Type-236 DR_DONE PI2. The v1585 timer observer is extended
in a disposable snapshot copy to select two **different** original
SLEEP/timer-tagged, VM-on-DISK descriptors owning real backing.
Without queueing RUN or calling swap service, it changes both
deadlines to the same next original PI6 clock tick. Original timer
service, scheduler, MM and physical I/O run unchanged thereafter.

Both separate full boots identify slots **6 and 7** as retimed,
with EXACTLY one original PI6 wake for each during snapshot. Each
boot completes five physical mapped BACKSTORE reads, 1,120 matching
Type-236 swap PI2 completions and four genuine successful resident
publications. Slot 6 suffers exactly one controlled checked-release
refusal after original READ/MM unpin, and the original
`proc_swap_in_free_fail` releases new MM, retains the original DISK
owner/bitmap/BCACHE identity, clears transition and delivers TERM.
The snapshot later commits normally on both boots; boot two first
restores prior first-boot MEMFS/XIP data and LOGSTORE record. The
original physical v2 header remains at DR3 `017777`, 3,648-word
pool and 277 other nonzero backing sectors. Original D6FSR2
descriptor geometry remains intact.

Independent full post-HALT verifier checks original linked labels,
not guessed historical addresses, real owner slots/wakeups, full
rollback owner fields, MM extent/table, backing bitmap, process
state, BCACHE identity, Type-236 PI2 and four physical images.
Sixteen deliberate wrong PI6/timer/owner/MM/TERM/bitmap witnesses
are rejected, plus 13 original physical v1584 mutations: **29**
unsafe cases rejected. v1592 testkit report and reproducibility:
`daimos-testkit/tests/validation/daimos-p1c-two-owner-physical-v1592.md`.

**Limit:** four original successes are aggregate; v1592 does not
read the exact per-slot RESIDENT publication identifier for slot 7.
Do not claim the specifically woken counterpart published until
that original owner is independently observed. Original owned
BACKSTORE controller fault remains fatal: this test injects a
recoverable checked provider release refusal before changing its
bitmap, not a magnetic-media error. P1-C further needs mixed
DEVICE/VOLUME/DTFS fault timing, dirty/write-through isolation
and repeated loaded-simulator stability. v1591 physical P1-D
alternate inventory passed separately; P1-E remains deliberately
on hold for KA10-aware architecture. No new production kernel
code/RAM/ABI/format, VERSION 0.5 and strict 32K optimization HOLD.
