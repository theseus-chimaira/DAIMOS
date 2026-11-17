# P1-E v1651: source-linked physical 48K MM occupancy checkpoints

Date: 2026-11-17. **Original physical 48K checkpoint accounting
PASS; original v1643 FULL source/read-failure auditor PASS.**
v1651 does not contain a new PDP-6 boot or continuous sampler.
No production KCORE, MM, MRES, VFS, PURE, scheduler, ABI or
permanent RAM change.

## Original physical source and geometry

The input is the already accepted, source-built v1643
48K/no-AUX/PDP-6/TSFS-root guest with actual secondary
DTC3/DTFS compressed-PURE executables, six sleeping children,
seventh rejected RUN, real DTC3 read fault, successful
same-reel restore, exact EXIT/WAIT and normal HALT.

The MM extent table is original PDP-6 sorted two-word
physical descriptors, not MEMINFO's logical process counter.
All stage records are reconstructed with the accepted
v1638 parser, checked to be complete, nonoverlapping, in
the original physical arena and consistent with exact MM
type/size/pin/owner geometry.

The only managed arena is **042561..140000 octal**:
**31,375 decimal words**. The 48K physical memory limit
is 49,152 words. The lower 17,777 physical words are
outside this MM managed arena; this does not attribute
those words individually to KCORE, boot, MRES or scratch.
Installed MRES and other fixed-core memory are not a
separately measured per-component peak here.

## Original physical checkpoints

All quantities are decimal 36-bit physical **words**.
`largest` is the largest free contiguous MM gap. The
optional 02000-aligned usable remainder is not additional
memory: it describes how much of the best free gap lies
after an actual 1,024-word APR alignment boundary.

| Stage | Occupied | Total free | Largest gap | Max 02000-aligned remainder | Dynamic MM words |
| --- | ---: | ---: | ---: | ---: | ---: |
| B | 27,163 | 4,212 | 883 | 640 | 2,331 |
| A | 27,163 | 4,212 | 883 | 640 | 2,331 |
| FULL_PINNED | 27,668 | 3,707 | 883 | 640 | 2,196 |
| ROLLED_BACK | 23,572 | 7,803 | 4,096 | 4,096 | 2,196 |
| LATER_FULL | 27,668 | 3,707 | 883 | 640 | 2,196 |
| Z | 4,958 | 26,417 | 26,417 | 25,600 | 862 |

Every stage satisfies `occupied + free = 31,375`.
Original B and A contain six distinct unpinned
`MM_TYPE_PACKED_PROCESS` suffixes of **3,456 words** each
and the unpinned 4,096-word parent PROCESS. In FULL_PINNED,
one earlier plain child has already exited: five remaining
PACKED suffix owners, a new **4,096-word pinned** full
PROCESS destination and its simultaneously pinned RAW
source, and the original unpinned parent PROCESS.

The occupied difference B/A -> FULL_PINNED is only
**+505 decimal words**, not +4,096: one 3,456-word
RAW suffix owner disappeared when the earlier child exited,
and the KERNEL_DYNAMIC allocation total also declined
by 135 words. Exact identity:

`4096 - 3456 - 135 = +505`.

At the two-pin FULL checkpoint, although **3,707**
managed words are free in total, the largest gap is
**883 words**; none can fit even a single
`02000`-aligned minimal PROCESS APR. The largest
available extent of an existing free gap after alignment
is only 640 words. It would be incorrect to infer
another runnable helper could be launched at that moment.

The real v1643 read failure frees exactly the 4,096-word
temporary target without destroying the 3,456-word
authoritative suffix. The original ROLLED_BACK stage has
4,096 more free physical words than FULL_PINNED, and
an exact 4,096-word free gap is restored. Later FULL
reuses a complete target for a different compressed
owner, reaching the same total allocation. After the
six exact WAITs, Z contains one parent PROCESS and
seven dynamic extents; no child PROCESS/PACKED owner
remains.

## Validation and false witnesses

The v1651 testkit runner first reruns the entire accepted
v1643 original physical read-failure source-linked audit
including its read/pin falsifications. Its second auditor
verifies the explicit MM arena and all six checkpoint
geometry/occupancy/counts. Three separately forged
copies of the physical FULL_PINNED block must fail:

* A fake one-word managed-arena extension at its lower bound.
* An understated one-word KERNEL_DYNAMIC extent.
* A false unpinned state on the complete FULL target.

All are correctly rejected by the physical accountant.
No simulator, KCORE, MM or original trace is modified.
Testkit:
`tests/diagnostics/pure-peak-ram-v1651/`,
branch `test-kcore-p1e-physical-ram-v1651-20261117`,
commit `9815f0c`.

Versioned complete evidence:
`$TMPDIR/daimos-p1e-v1651-evidence-20261117.tar.gz`.
SHA256:
`248842c351f3d5070ee8485a1691571fd5ee088b8bb1a5400c11b668cba109a0`.
It includes source-linked original physical maps, 48K trace,
original and rejected audit logs, and a SHAR whose final
action performs git commit/push.

## Precise acceptance limit

**ACCEPT:** sampled original v1643 48K MM occupancy/free
counts, physical two-pin owner shape and exact bounded
FULL rollback geometry. The largest of the six observed
allocations is **27,668 words**.

**NOT PROVEN:** that this is the whole-machine continuous
transient peak (there are unsampled times and storage
outside the managed MM arena); matched PURE-on vs PURE-off
net gain for this larger v1643 guest; exact 32K or 64K
BCACHE/MRES profiles; distinct KA10 VM. v1639's already
accepted +1,314 decimal-word net savings for SIX PACKED
owners belongs to a different, source-matched 48K ON/OFF
comparison and cannot be silently transferred to v1643.

P1-E still OPEN, PURE OFF by default, strict 32K KCORE
optimization HOLD.
