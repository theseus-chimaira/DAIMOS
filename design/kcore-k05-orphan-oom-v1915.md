# K05 v1915: genuine requester-only OOM under quarantined foreign FULL

Date: 2026-11-24. **Scoped original 48K PDP-6 physical PASS.**

## Problem, scope and original allocator policy

v1914 physically accepted original 17-bit retry-clock wrap
under a last-live foreign-owned FULL orphan. The next K05
boundary is allocator pressure from a SECOND original user
while that foreign target/RAW owner is quarantined. The
requester may be rejected, but its error handling must not
reclaim the unrelated foreign child's still-live private
executive stack or its authoritative RAW source.

This v1915 test deliberately isolates **physically impossible
BRK arena-capacity refusal**, NOT a general fragmented MM
allocator or descriptor exhaustion. The original
`vm_space_brk_current` first calls `mm_arena_fits_v1260` for
the aligned target APR extent. A size which can never fit
in any physical arena must fail with genuine MM NOMEM before
destructive victim swapping, compaction or source release.
The original `native_sys_brk` then invokes its already-present
requester-only `proc_fatal_close_current_v879` policy.

## Original source-built guest with user-only test gate

The testkit source-builds a v1907-derived C89 original parent
USER APR, reusing the existing test-only `v1901_term_gate`.
One additional user-mode `dsys_brk(0130000UL)` is armed only
AFTER the original PACKED-source mm_free genuinely returned
-5, the provisional full target was physically test-claimed
by foreign owner2, and the parent's real TERM(3) was refused.
The guest prints `P` and makes the original syscall; any
returned call prints `!O` and fails the physical test.

The original fixed KCORE and optional PURE DXR are completely
byte-identical to the accepted v1906-v1914 objects. No
production C/ASM or resident KCORE RAM change occurs. Only
the test-only USER executable and source-built compressed
TSFS metadata differ. The 54-file compressed medium is
independently verified before PDP-6 boot.

## Physically observed native capacity refusal

Original `mm_arena_fits_v1260=002154` receives AC1=`0130000`
octal words and AC2=`002000` alignment, a legal 18-bit BRK
request. The original physical MM table reports exactly one
managed arena, `044633..0140000` (size `073145` octal
words); after `002000` alignment its largest interval is
only `072000` words. The requested `0130000` APR is therefore
physically impossible regardless of available victims, not
merely fragmented. The native caller receives real MM NOMEM,
and original `proc_fatal_close_current_v879=013713` executes
with AC1=1 and current parent slot1.

Original `proc_exit_current=013655` then completes parent
fatal EXIT, returning control to permanent slot0 idle.
The complete MM18 table is unchanged at the native arena
check, OOM entry, and parent EXIT entry. The parent's normal
finalizer reclaims ONLY `045761..046371` KERNEL_DYNAMIC
owner `01001` and `130000..140000` PROCESS owner1: MM18
-> MM16. Foreign slot3's RAW `111200..120000` owner3 pin0,
provisional full PROCESS target `120000..130000` owner2
pin0, private CTX_PC `340000016704`, FDCTL `000000004010`,
uarea[067] exact target escrow `040000120000`, TRANSITION
and ROOT are unchanged.

The real healthy child users in slots 4,5,6 then enter
three DISTINCT original native SYS_EXIT calls separated by
original idle-loop completions. Their exact six MM extents
are released, no others: MM16 -> MM10. Slot3 is the ONLY
LIVE orphan, parent0, original descriptor `047017600001`,
VMbase0, with all other user slots FREE. Original native
`proc_finish_service_v881=013065` then executes three
more real capped retries at physical PCLK `024075`, `025075`,
`026075` octal, exactly `01000` ticks apart. Shared pending
and retry delay `01000` stay armed through every scan.
Original PCLK PI6 saves `340000017565` (interrupted original
slot0 idle). The PDP-6 never enters native guest HALT;
simulator ends by explicit operator command.

This is an MM capacity **early rejection**: the proof does
NOT claim a successfully entered general `mm_alloc_aligned`
fragmentation/reclamation path under the same failure.
K05 still needs independent general allocator/compaction and
descriptor-error validation to close this wider risk.

## Strict physical audit, rejected controller experiments

The accepted source-linked verifier checks original binaries,
USER gate and pressure arguments, original arena geometry,
the native OOM/EXIT PCs and ACs, exact MM18/16/10 owner
deltas, ROOT and child continuation invariance, the complete
seven-user process table, three original capped retries,
and genuine idle PI6. Four unsafe physically-mutated traces
all reject: wrong BRK target, false foreign owner transfer,
lost private escrow, and prematurely FREE orphan.

An initial v1915/v2 physical script observed only through
process slot5, omitting slot6 descriptor word `045722`.
The strict verifier correctly rejected the incomplete
snapshot. The next v1915/v3 script mistakenly treated the
linked KCORE `proc_table` symbol at `024635` (a POINTER)
as the physical process table itself; its address range
was also rejected. The accepted v1915/v4 script derives
the original physical table base `045700` from the source-
linked original v1906 fixture, verifies the live pointer
contents, and reads all original descriptors through
`045726` at EVERY native checkpoint. Both earlier runs
were discarded, never evidence of a kernel defect; this
was test instrumentation debugging, not a workaround.

Accepted normalized physical transcript SHA256:
`fc8ac14b4c1a31507d974982fb624a52abad72f2d15250c6683a609d9ea53d58`.
Source-linked physical controller SHA256:
`d0a6860c3fc8fb4979ec4f73dccd84d0d17f26281e8a7de36ed701e69b995bfc`.
Original fixed KCORE SHA256:
`a201a7fdcf481b436b06694e5dd55c9367036ebd9f64c630486f854b1a4f89be`.
Original optional PURE SHA256:
`09f65d0ca272986b462c63580eae3c4006490004cd060529ce40f10a55949351`.

Testkit: `daimos-testkit/tests/validation/pure-k05-orphan-oom-v1915/`.
Permanent docs: `pdp10-doc/KERNEL/PURE-K05-ORPHAN-OOM-V1915.MD`.
Evidence: `$TMPDIR/daimos-k05-orphan-oom-v1915-physical-v4/`.

**K05 OPEN:** this is a physically impossible APR capacity
request while the quarantined child is not yet the sole live
user. The requester exits, leaving that child the sole LIVE
orphan. General allocation failure, compaction, descriptors,
other PI6/allocator interleavings and cross-platform release
still need separate acceptance. K06-K09 OPEN. RAW PURE and
mode3 default OFF. No resident RAM or fixed binary growth.
