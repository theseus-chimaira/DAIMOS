# KCORE P1-C v1583: physical mapped VOLUME snapshot PI2 fail-stop

Date: 2026-11-15. **ACCEPTED** full 128K PDP-6 physical four-member
Type-236 VOLUME snapshot-header failure with live process swap owners.
VERSION 0.5; strict 32K/optimization HOLD. No production source, ABI,
format, process-table or resident RAM change.

## Existing evidence and new scope

The earlier v1547/v1548 acceptance already demonstrated actual PI2
BACKSTORE-first and ROOT-first failures on a booted four-member DR0..
DR3 D6FS root, with owned swapped backing. v1582 connected the real
compact BLOCKSET mapper to the fixed BACKSTORE gateway under native
PDP-6 test-controlled hardware completions. v1581 established real
MEMFS persistent snapshot-header failure on direct AUXSTORE-backed
Type-236 DRM0. **v1583 closes the missing intersection:** the full
PDP-6 MEMFS snapshot shutdown itself selects a physically mapped
multi-member VOLUME tail while real swapped processes still own
BACKSTORE blocks, and the original PI2 error goes through the
production critical-failure classifier.

The disposable full-guest image uses the production `mkdsk`/`mkd6fs`
four-member `BOOT=drm ROOT=drum ROOTSET=1` recipe, actual root D6FS,
LOGSTORE LSREC1, persistent MEMFS data/XIP and the v1580 four sleeping
24K-word private-image children. The original fixed KCORE process
swap owner count is required to be positive when the actual snapshot
shutdown begins. The accepted v1581 test-only injector selects the
first invalid snapshot-header WRITE at the last logical BACKSTORE
block, binds its *actual* private request descriptor and waits for a
real Type-236 DR_DONE PI2 completion before redirecting that one
descriptor to the unchanged original driver error leaf. No negative
I/O result is forged at the BACKSTORE gateway.

The actual guest entered the original BACKSTORE-specific HALT at
octal PC `004011`, not the ROOT leaf. At physical HALT the simulator
reported these original and diagnostic linked KCORE words:

| Witness | Observed octal |
| --- | --- |
| v1581 test-selected PI2 error count | `000001` |
| original DRM error counter (+5) | `000001` |
| original BACKSTORE block ownership | `001374` |
| active BACKSTORE enabled | `000001` |
| selected relative snapshot tail sector | `001777` |
| mapped VOLUME filesystem-data sectors | `075774` |
| original BACKSTORE span and capacity | `002000` |
| `device_region_map` | `400000000000` |
| actual D6FS member-set packing | `032104` |
| actual D6FS root vnode | `060101000000` |
| selected hardware group | `777770` |
| retired active target descriptor | `000000` |
| cleared selected-request arm | `000000` |

The independent host verifier parsed all four genuine 8192-sector
Type-236 raw 36-bit images and each D6FSR2 unit-zero descriptor.
All four members have base `1`, usable `017377` and swap tail `0400`
octal sectors; every tail ends at physical sector `017777`.
The final relative mapped BACKSTORE sector `01777` goes through the
unchanged four-way INTERLEAVE mapper to **DR3 block `017777`**, whose
Type-236 group address `(3 << 16) | (017777 << 3)` equals `777770`.
After the error, that physical sector's snapshot commit header is
still invalid (zero). **248 other nonzero sectors** remain in the
four BACKSTORE tails in the final accepted fresh run.
The actual per-member D6FSR2 descriptors were byte-identical before
and after the fatal HALT. A DIRTY D6FS superblock after this abrupt
intentional HALT is expected: the initial test mistakenly treated it
as an invalid root, corrected to compare immutable physical layouts.

All 12 independent witness/media mutations were rejected: missing
test PI2 or original driver accounting, lost owner, direct-DEVICE
misclassification, wrong volume capacity, wrong Type-236 group,
unretired descriptor, wrong HALT PC, wrong root member map or mount,
falsely committed snapshot header and corrupt member descriptor.
The single successful full-guest rerun, strict host oracle and its
mutation matrix all returned exit status zero.

## Acceptance boundary

This is test-induced selection of an existing PI2 error branch after
a real simulated DMA completion; no natural media damage is claimed.
An outstanding swapped child is not evidence that an independently
scheduled process completes PI-driven swap-in *during* synchronous
snapshot I/O. A fatal HALT cannot prove successful recoverable
BCACHE/MM/process-owner rollback or health of remaining VOLUME
members on continued execution. **P1-C remains OPEN** for these
independent cases. P1-D alternate physical inventories and P1-E
uncompressed PURE are also open. P0-A and trusted-operator P0-B
remain closed. Fixed KCORE low end remains octal `024364`.

Reproduce with the testkit's versioned
`tests/validation/daimos-p1c-mapped-snapshot-pi2-physical-v1583.sh`;
its accompanying `.py` verifier and `.md` test report contain the
fixture and raw-evidence detail. Set `V1583_KEEP_WORK=1` to retain
both source-linked binary and physical drum images.
