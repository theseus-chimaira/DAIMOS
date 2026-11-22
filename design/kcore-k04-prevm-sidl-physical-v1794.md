# K04 v1794: PREVM A / unpublished SIDL B physical recovery

Date: 2026-11-22. Status: **PASS** for the reverse physical
PREVM-A/SIDL-B ownership ordering, B's independently armed first
retry, both real MM releases, safe PID reuse and clean guest HALT.
K04 as a whole remains OPEN.

## Purpose and scope

Previous v1793 passed SIDL A / PREVM B. This test reverses the
stages while preserving the ordinary provider paths:

* A (PID 5) enters EXIT PREVM through the real private owner stack,
  retaining its exact VM MM extent until its 24th actual
  `vm_space_destroy` attempt (octal 030). Real `mm_pin` and
  `mm_unpin` apply to the saved VM extent; ordinary VM destruction
  is not replaced.
* B (PID 6) is an unpublished RUN child. Its first RUN hierarchy
  operation takes the existing error rollback and real SIDL
  discard path, not the EXIT path. Its own VM extent is physically
  pinned through four refused `vm_space_destroy` attempts, then
  unpinned on its fifth attempt.
* B must start at an octal 020 interval despite A's saturated
  octal 01000 shared delay. A must remain PREVM, still physically
  refused, while PID 6 becomes reusable with no teardown fields.
  A may only become reusable after its own exact VM release.

The disposable 256K Open SIMH PDP-6 guest was derived from the
working v1784 post-VM/SIDL B fixture. Only the generated scratch
`kcore-asm/proc_pdp6.s` was changed to replace post-VM A with
the validated real PREVM A fixture from v1782; the generated
`proc_run.s` retains the proven one-time PID 6 SIDL rollback
hook. The fixture **preserves the normal POPJ** at the end of
`proc_finish_mark_pending_v892`, whose accidental deletion
caused the rejected v1788 scratch-only UUO regression.

No fake provider error, no scheduler override, no production
DAIMOS kernel source or ABI change, and no additional fixed
KCORE RAM are involved. All added assembly and BSS are restricted
to this disposable test image.

## Build, instrumentation and initial media

Versioned scratch image:

    $TMPDIR/daimos-k04-v1794-prevm-sidl

The versioned source generator is in `daimos-testkit`:

    tests/validation/daimos-k04-prevm-sidl-fixture-v1794.py

It requires two generated `kcore-asm` directories, transforms
the v1784 generated scratch files, and reads v1782 as the
PREVM A source. Running it independently in a second generated
scratch directory produced byte-identical `proc_pdp6.s` and
`proc_run.s` compared to the actual v1794 image sources.

Native DAS PASS, full 256K image build PASS, initial read-only
`d6fsck -n 4` superblock 742 seq=1 CLEAN PASS. Simulator
execution was at reduced priority with two ROOT/DSH guest
terminals (CTY and DCS0). The disposable `boot.ini` used DPY
HEADLESS and disabled only the optional WCNSLS SDL graphical
console: the first exploratory start otherwise could not open
an SDL window on the host. CTY, DCS and the guest kernel remained
enabled. The graphical-host start is not a KCORE fault result.

## Live process ownership and first-refusal timing

CTY runs `TRUE` first, holding PID 5 on its private EXIT PREVM
stack. After A has reached its capped retry interval, DCS0 runs
`TRUE`, which deliberately fails its first PID 6 RUN hierarchy
and leaves B as unpublished SIDL under real VM pin refusal.
The DCS shell returns before the deferred SIDL recovery is
complete: shell completion alone must **not** be mistaken for
PID 6 reuse.

The first exploratory test sampled `PS` immediately after B
returned; it properly showed both owners still refused. That
run was not accepted as the complete independence gate. The
corrected driver polls DCS `PS` until PID 6 is actually reused
while A remains PREVM, then waits for A's own release.

In the accepted run, DCS `PS` showed both facts in the same
snapshot:

    PID PPID S WORDS TD RETRY PVM DETAIL ERR COMM
    5   2    2 2048  3 256   1      0   0 TRUE
    6   3    2 3072  0   0   0      0   0 PS

A remains a live-VM PREVM stage-3 finalizer with PVM=1; B has
already been reaped and PID 6 reused with every teardown field
zero. This provides the reverse-stage concurrent-owner evidence.

## Post-HALT physical memory witnesses

CTY's original `TRUE` returned only after A's real MM recovery.
The subsequent CTY `PS` reused PID 5 with TD/RETRY/PVM/DETAIL/
ERR all zero. CTY `HALT` stopped at PC octal 022621; the SIMH
command file examined the scratch BSS only **after** this
guest-initiated HALT and then exited normally.

Physical values in the accepted scratch map (all octal):

    025126: 000000000000  ; B SIDL exact VM base released
    025127: 000000000005  ; B five real VM destroy attempts
    025130: 777777777777  ; B real mm_unpin completed
    025131: 000000000000  ; A PREVM exact VM base released
    025132: 000000000030  ; A 24 real VM destroy attempts
    025133: 777777777777  ; A real mm_unpin completed
    025134: 401000004625  ; before B: A pending at capped 01000
    025135: 400020005265  ; after B: new pending 020 interval
    025136: 000000005265  ; B first real refusal clock
    025137: 000000005305  ; B second real destroy-attempt clock

The first qualified B retry interval is exactly
`005305 - 005265 = 020` octal ticks (16 decimal), independent
of A's prior capped interval. Both real VM extent pins released,
and both slots were safely reused in the required order.

Final read-only `d6fsck -n 4` selected superblock 742 seq=3,
state=CLEAN, with 179196 blocks and 360 FCBs.

## Testkit evidence and limitations

The physical driver and read-only independent validator are in
`daimos-testkit`:

    tests/validation/daimos-k04-prevm-sidl-run-v1794.py
    tests/validation/daimos-k04-prevm-sidl-verify-v1794.py

The driver performed two guest ROOT logins, both TRUE operations,
repeated actual `PS` snapshots, safe guest HALT and read-only
disk validation. The separate verifier accepted the real map,
CTY/DCS/SIMH transcripts and clean D6FS. Its positive self-test
and eight intentionally corrupted negative controls all PASS.

Both physical SIDL/PREVM owner orderings are now accepted:
SIDL-A/PREVM-B v1793, and PREVM-A/SIDL-B v1794. Other K04
coverage remains OPEN: FILE-close/SYNC refusal, session HUP,
dying-parent WAIT/PID reuse, permanent MM refusal/operator
recovery, and any additional mixed-owner clock-wrap stress.
The earlier single-owner SIDL physical clock-wrap v1787 passed
separately. Do not mark overall K04 complete.
