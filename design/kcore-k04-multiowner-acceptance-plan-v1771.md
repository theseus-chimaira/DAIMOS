# K04 v1771: physical multi-owner acceptance plan

Date: 2026-11-21. Status: source-audited test design only;
**no fault-injected execution has been performed**. The v1770
256K PDP-6 boot, login, normal child and clean-reboot baseline PASSED.

## Source-backed ownership paths

The deferred reaper is `proc_finish_service_v881` in
`system/kernel/proc/proc_pdp6.s`. It scans only TRANSITION slots,
distinguishes SIDL with the `02` discard-VM retry tag from an
ordinary FINALIZE owner, and dispatches the former to
`proc_slot_discard`. Other eligible owners need a stable nonzero
u-area and a FINALIZE marker. A non-NONE VM additionally needs
PREVM; VM_NONE with DISCARD still uses discard rather than EXIT.
Failed cleanup sets bit 1 of the scan result. Only a scan without
any failed owner clears the shared pending retry delay.

The shared retry word `proc_finish_last_tick_v881` carries a
17-bit clock epoch, a pending flag and a delay starting at octal
020 ticks and doubling to octal 01000. The slot-zero idle path
advances the old interval before the owner scan (v1765). A
newly armed FINALIZE owner uses the existing word 067 ARM bit
(v1763) to reset the shared interval independently of another
owner. A SIDL rollback uses the scheduler word's discard-VM tag
to identify its first refusal. Both the scheduler's due check
and the idle loop subtract epochs modulo the 17-bit clock mask.

The reaper does not execute FILE close or session HUP. Those
steps belong to the private owner stack before the finalizer
switches to slot zero. PREVM retry revisits TTY release and VM
destroy; post-VM retry releases only the retained u-area. A
successful finalizer may publish a ZOMB only for a WAIT-capable
parent; otherwise it adopts to eligible INIT or frees the slot.

## Required real PDP-6 scenarios

1. **Single temporary MM refusal:** pin the exact VM or u-area
   MM extent, start EXIT, observe PREVM or post-VM owner and a
   nonzero retry delay; release the pin, observe eventual FREE or
   ZOMB/WAIT and no remaining MM ownership.
2. **Permanent MM refusal:** keep the exact extent pinned across
   multiple 020/040/0100/.../01000 intervals. Confirm no busy
   loop, no premature PID reuse, no freed descriptor and no
   unrelated scheduler starvation. Stop through a controlled
   operator procedure; do not forcibly terminate a writable D6FS.
3. **Two independent owners:** hold A in permanent refusal,
   create B and refuse its first cleanup. B must start with its
   own first-refusal 020 interval rather than inherit A's
   saturated delay. Release B while A remains refused and verify
   B completes, A remains owned and the shared pending flag stays
   set. Repeat with B in SIDL DISCARD and with PREVM/post-VM
   owners in both orders.
4. **Clock wrap:** seed the qualified 17-bit APR clock near
   octal 0377777, arrange pending cleanup across the wrap,
   verify elapsed modulo arithmetic and capped delay. The
   clock must be controlled in a disposable instrumented image,
   never by changing production timer policy.
5. **Session HUP:** refuse one member's release after HUP is
   committed, then exit the leader while another member is
   still live. Verify no repeated HUP to an irreversible owner,
   no premature TTY release and eventual retirement when the
   last actual member exits. Include a VM_NONE DISCARD member.
6. **WAIT and PID reuse:** kill the parent before its child
   finalizes. With healthy INIT, verify adoption and a single
   terminal WAIT; with INIT unavailable, verify direct cleanup
   and no uncollectable ZOMB. Reuse a slot only after its old
   exact MM/FILE owner has released it.
7. **FILE refusal:** force a provider close/SYNC error while
   the owner is on its private executive stack. Ensure the
   live descriptor remains owned and the slot is not handed to
   the non-sleeping MM reaper until FILE and HUP finish.

## Evidence gate

Each scenario needs the actual PDP-6 console transcript,
per-slot GETTEARDOWN/PS observations, provider/MM owner state,
relevant elapsed clock ticks, and a clean guest shutdown with
read-only D6FS verification. Fault-injection fixtures and all
tests belong in `daimos-testkit` once the external testkit sweep
is available; do not add them to production DAIMOS or alter the
concurrent suite. Until those observations exist, K04 remains
OPEN regardless of the v1770 production boot PASS.
