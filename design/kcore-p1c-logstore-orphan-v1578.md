# KCORE v1578: physical LOGSTORE orphan skipped across cold boot

Date: 2026-11-14. A P1-C storage fault-recovery increment. Overall
P1-C remains OPEN. P0-A and operator-trust P0-B remain CLOSED.
VERSION 0.5; strict 32K optimization HOLD.

The v1577 case physically wrote an uncommitted LSREC1 with its final
trailer zero but retried and committed it *before* a cold boot. v1578
extends that case to an **abandoned** invalid record. The first full
128K KCC/PDP-6 Type-236 guest sends a unique LOGCTL APPEND. A
disposable copy of the real LOGSTORE MRES physically writes its
correct 128-word record with a zero final commit trailer and reports
an error; the original append state stays at its previous cursor.
The user test checks the unsuccessful status, unchanged cursor, and
actual uncommitted physical sector through LOGCTL READ_BLOCK. The
test does NOT retry; privileged HALT commits the unrelated persistent
MEMFS data and XIP image on the same 8 MiB AUXSTORE drum.

Before second boot, host parsing of real physical Type-236 media
locates LOGSTORE through the published AUXSTORE descriptor and
independently confirms exactly one invalid first-boot LSREC1 at
octal physical block `05374`, sequence `3`, with the expected
36-bit payload and a **zero** commit trailer. It is not within the
BACKSTORE snapshot allocation.

The second full cold boot uses fresh CPU RAM and the exact same
drum image. Production KINIT's LOGSTORE recovery must disregard
this invalid record, instead of advancing the cursor past it.
The new boot restores the original persistent MEMFS file/XIP,
verifies no *committed* first-boot test event exists, appends and
reads an independent valid second-boot event, and HALTs normally.
Independent final physical inspection proves KINIT *reused* the
original invalid block/sequence rather than skipping it. Under the
fixed INIT/LOGIN test profile, successive physical slots contain
complete, committed LSREC1 sequences `3,4,5` (the second test
event is sequence 5). A bad recovery that counted the invalid
orphan would begin at 4 and fail this oracle. The AUXSTORE
descriptor remained byte-identical across both boots; both probes
exited STATUS:0.

Four physically altered images are rejected: false first-boot event
commit, skipped first reused sequence (valid recomputed checksum),
skipped second-boot test-event sequence (valid recomputed checksum),
and missing final commit trailer. The current source-linked native
`daimos-logstore-v1.sh` suite also passes LOGSTORE recovery, torn
record, drain and loss regression. All new code and fault counters
reside in daimos-testkit/disposable MRES; production KCORE and all
services are byte-for-byte unchanged by v1578. No new fixed RAM,
process table, MM, BCACHE, ABI, module or on-media format.

**Important limit:** the physical invalid sector is deliberately
written by the test's real Type-236 DMA and the negative completion
is injected afterward by an isolated optional-MRES hook. The
controller itself did not naturally fail or tear a sector. The
recovery is real KINIT on real retained physical media, but it is
not yet a full natural-hardware-fault acceptance. Nor did HALT
overlap an independently demonstrated physical process swap-in/out:
`memfs_snapshot_shutdown` runs synchronously inside the privileged
HALT syscall, so a resident/dormant swapped child is insufficient
evidence of concurrent commit. P1-C still requires a timed full-
guest swap/commit overlap and PI/DEVICE/VOLUME fault isolation.

Executable testkit:
`tests/validation/daimos-p1c-orphan-reboot-v1578.sh/.md`,
`daimos-p1c-orphan-media-v1578.py`,
`daimos-p1c-orphan-recovery-v1578.py`,
`daimos-p1c-orphan-mutants-v1578.py`.

The original unmodified v1576 two-cold-boot positive LOGSTORE/MEMFS
physical regression was rerun after tightening the shared guest
commit-trailer validity check and again PASSed both boots, record
recovery and descriptor equality. This is a separate normal-record
regression, not evidence of a new hardware-failure path.
