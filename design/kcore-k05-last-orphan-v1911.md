# K05 v1911: last quarantined FULL owner prevents premature native HALT

Date: 2026-11-24. **Scoped original PDP-6 48K physical PASS.**

## Last-user question

v1910 proved that real parent EXIT safely releases exactly its
own PROCESS and uarea MM extents, adopting the foreign-claimed
FULL child as parent0 without clearing TRANSITION or its saved
private executive continuation. It did not prove that the child
would prevent final-process HALT after every healthy sibling
exited. v1911 extends that SAME original source-built guest,
KCORE, PURE MRES, and source-linked simulated foreign-claim
fixture, without another production build or new RAM state.

After original real PACKED-source `mm_free=-5`, temporary
PROCESS target owner3->foreign owner2, real refused parent TERM,
and original parent SYS_EXIT, the original non-quarantined
siblings really execute native `proc_exit_current=013655`
in slots **4, 5 and 6**. Three individual real slot0 idle-loop
checkpoints ensure a previous child finalizer completes before
the next exit breakpoint can fire. Original parent slot1 and
already-reaped slot2 remain free.

The final `proc_idle_loop=017552` checkpoint shows only slot3
live: `proc_high_slot=4`, `proc_current_slot=0`, slots1,2,4,5,6
zero. MM count drops **16 -> 10**, removing only three child
KERNEL_DYNAMIC uareas, owners `01004..01006`, and three child
PACKED RAW suffixes, owners4..6 (exact six source-linked extents).
There is no other MM insertion, deletion or owner mutation.
The original slot3 descriptor remains `047017600001`, orphan0,
TRANSITION, VMbase0, with RAW source `111200..120000` owner3,
still-present foreign full PROCESS target `120000..130000`
owner2, original CTX_PC `340000016704`, FDCTL
`000000004010`, and u[067] escrow `040000120000`.

The source `proc_has_live_user` scans the compact scheduler
state and finds the live TRANSITION-held owner. The physical
observation proves original idle rather than final-process
HALT: after the last ordinary child exited, there were three
distinct native `proc_finish_service_v881=013065` retry calls,
interleaved with original idle-loop returns. Their real PCLK
values `020046`, `021046`, `022046` octal are separated by
exactly `01000` ticks (512 decimal), and the original shared
delay stays capped at `01000`. All 10 remaining physical MM
extents and original ROOT/context stay invariant. One further
genuine original PCLK PI6 at `017225` saves interrupt word
`340000017617` (PC `017617` inside the original idle loop),
with unchanged slot3 and MM10.

Exactly three focused physical trace mutants reject a false
live sibling, missing pending retry marker, and lost child
escrow after the final PI6. KCORE and PURE unchanged; no
added per-process state, RAM, IMAGE or BSS.

The first v1911/v1 experiment was correctly rejected because
three SIMH monitor stops occurred at one unexecuted child EXIT
PC after rearming the same breakpoint. The valid v1911/v2
controller forces a separate original idle-loop return after
every child EXIT and after every retry entry. Do not mistake
simulator monitor labels or an operator EXIT for native HALT.

Original fixed KCORE SHA256:
`a201a7fdcf481b436b06694e5dd55c9367036ebd9f64c630486f854b1a4f89be`.
Optional PURE SHA256:
`09f65d0ca272986b462c63580eae3c4006490004cd060529ce40f10a55949351`.
Accepted normalized physical SHA256:
`2863df28f5712639a8c42ecce7c6b6566cd9328f06cbbee7809f2642dd60ff9c`.

Testkit: `daimos-testkit/tests/validation/pure-k05-last-orphan-v1911/`.
Permanent docs: `pdp10-doc/KERNEL/PURE-K05-LAST-ORPHAN-V1911.MD`.
Evidence: `$TMPDIR/daimos-k05-last-orphan-v1911-physical-v2/`.

**Scope boundary:** this establishes one real last-orphan
ordering and three capped native idle retries, not guaranteed
infinite-time safety or safe forced terminal cleanup. The
foreign-owned full target and live executive continuation
must NOT be forcibly freed merely because no ordinary users
remain. K05 remains OPEN for terminal policy and remaining
allocator/PI6 cases; K06-K09 OPEN, RAW PURE/mode3 default OFF.
