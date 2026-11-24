# K05 v1910: native parent EXIT while foreign FULL child stays quarantined

Date: 2026-11-24. **Original 48K PDP-6 physical PASS, scoped.**

## Ownership question

The v1906 escrow fix retains the exact provisional FULL target
base in the already-existing uarea word067. v1907-v1909 show
genuine rejected TERM, bounded pending backoff and one real
PI6 under a permanently foreign-claimed target. v1910 adds
ONE ordinary lifecycle question: may the parent exit and
release **its own** resources while a still-live child has
an unresolvable foreign-owned FULL target?

The answer is yes for this measured ordering. A source-built
original five-child compressed-TSFS PDP-6 guest performs
real PACKED source `mm_free` returning -5; the controller
changes only original provisional PROCESS target owner3 to
foreign owner2, and original parent user TERM(3) refuses (`TR`).
The controller then opens a new test-only USER gate. Original
parent prints `E` and executes `dsys_exit(0)`. The original
KCORE reaches `proc_exit_current=013655`, real slot0 owner
finalizer `proc_finish_slot_owner_v932=012610`, and original
idle loop `017552`, without touching the child's foreign target.

The parent metadata at `045703` moves from `045761400001`
to `045761600001` during real FINALIZE, then zero.
The original MM count 18 -> 16 deletes exactly two extents:
parent KERNEL_DYNAMIC uarea `045761..046371` owner `01001`,
and parent PROCESS `130000..140000` owner1. There are no
added or otherwise changed MM descriptors. Exact original
child meta `047017600401` -> `047017600001` proves
parent1 -> orphan0 adoption while TRANSITION remains set.
Its RAW suffix `111200..120000` owner3, foreign full target
`120000..130000` owner2, VMbase0, saved executive CTX_PC
`340000016704`, FDCTL `000000004010`, and uarea067 escrow
`040000120000` all remain intact.

A real original PCLK PI6 at permanent KCORE `017225`, with
saved interrupted idle-loop PC `017562` (`340000017562`),
leaves MM16 and all child rights and context untouched.
The operator then stops the simulator; this is NOT guest
HALT or a forced terminal reclaim of the still-live child.

The strict read-only source-matched physical verifier checks
both real parent/slot0 breakpoints, original SYS_EXIT,
original parent descriptor and two exact MM extent releases,
exact orphaning, preserved full MM table, PI6, and no
forced FINALIZE. Three targeted independent unsafe physical
mutants (false parent context, lost child TRANSITION, lost
child escrow) all reject. No duplicate broad test matrix.

## Distinguish test fixture failures from kernel behavior

One discarded attempt passed a simulator console port above
65535; another ran an interrupted build before its genuine
v1638 compressed `/SYSTEM/COMP` test media was generated,
so the original user guest correctly returned `!3`.
Both transcripts were rejected: their artificial follow-on
checkpoint labels were NOT native proof. The corrected
fixture requires actual complete compressed TSFS member
validation and a legal console port before physical execution.

The accepted physical trace SHA256:
`262998e8a03dcad8b96247dd6761661cd5a8668226e0dd6db6332ea94bc10a70`.
Fixed KCORE SHA256:
`a201a7fdcf481b436b06694e5dd55c9367036ebd9f64c630486f854b1a4f89be`.
Optional PURE SHA256:
`09f65d0ca272986b462c63580eae3c4006490004cd060529ce40f10a55949351`.
No production C/ASM, resident word, IMAGE or default policy changed.

Testkit: `daimos-testkit/tests/validation/pure-k05-parent-exit-v1910/`.
Permanent docs: `pdp10-doc/KERNEL/PURE-K05-PARENT-EXIT-V1910.MD`.
Accepted physical evidence:
`$TMPDIR/daimos-k05-parent-exit-v1910-physical-v3/`.

**K05 remains OPEN:** if the uncertain target cannot be
reconciled, no evidence supports discarding its still-live
private executive continuation. This test proves safe
*parent resource retirement*, not safe child retirement.
Unresolved allocator ambiguity and additional PI6 timings
remain; K06-K09 OPEN, RAW PURE/mode3 default OFF.
