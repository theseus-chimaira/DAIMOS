# K04 v1795: permanent physical VM refusal and safe operator HALT

Date: 2026-11-22. Status: **PASS** for permanently retained real
MM ownership, bounded deferred retries, independent B recovery,
no premature PID reuse, continued DCS scheduling and an orderly
guest operator HALT with D6FS CLEAN. Overall K04 remains OPEN.

## Physical scenario

The preceding v1794 experiment established both PREVM-A/SIDL-B
recovery and B's independent retry timer with two temporary
physical MM pins. v1795 retains the same validated source paths
but makes PREVM owner A (PID 5) *permanently refused*:

* Its first EXIT PREVM `vm_space_destroy` attempt pins the exact
  live VM extent with the real `mm_pin` provider.
* Every subsequent attempt validates the unchanged exact extent
  and invokes the unmodified `vm_space_destroy`, which genuinely
  refuses to free an extent that is physically pinned.
* Unlike v1794, this **disposable fixture has no A unpin path**.
  The VM remains pinned until the operator halts the guest.
* SIDL B (PID 6) has an independently pinned VM extent and uses
  normal SIDL discard and physical `mm_unpin` on attempt five.

This is neither a synthetic MM_ERR_BUSY return nor a policy
change to the deferred reaper. It does not alter production
DAIMOS source, resident kernel ABI, permanent DATA/BSS or KCORE
size. Instrumentation and its one additional count witness are
limited to the generated scratch assembly.

## Reproducible fixture and build

The clean scratch 256K PDP-6 image and versioned generator are:

    $TMPDIR/daimos-k04-v1795-permanent-prevm
    daimos-testkit/tests/validation/daimos-k04-permanent-pin-fixture-v1795.py

The generator transforms the validated v1794 generated
`kcore-asm/proc_pdp6.s` and `proc_run.s`. It removes A's
attempt-24 unpin, not the normal real VM-destroy call; the
mandatory `proc_finish_mark_pending_v892` return remains in
place. A new BSS word `k04_a_calls_at_b_v1795` captures A's
real destroy-attempt counter when B first pins its VM.

Native DAS PASS. Complete scratch image build PASS. Initial
read-only `d6fsck -n 4` selected superblock 742 seq=1 CLEAN.
The versioned fixture generator reproduced the tested assembly
byte-for-byte in a separate disposable replay directory.

The simulator ran with `nice -n 15`; optional WCNSLS/SDL
graphics were disabled only in scratch `boot.ini` to avoid
the host graphics issue. Both guest CTY and DCS0 ROOT/DSH
terminals remained online, together with the actual MM, VM,
clock, disk and scheduler services.

## Live guest evidence and bounded retry activity

CTY ROOT runs foreground `TRUE`, which enters EXIT PREVM and
never returns from its pinned VM cleanup. After the first
physical owner reaches its capped retry interval, DCS ROOT
runs `TRUE` to create a distinct unpublished SIDL B owner.

DCS `PS` is polled until B actually completes and reuses its
slot; the shell returning from the intentionally failed RUN
is not treated as completion. The first qualifying `PS` shows:

    PID PPID S WORDS TD RETRY PVM DETAIL ERR COMM
    5   2    2 2048  3 256   1      0   0 TRUE
    6   3    2 3072  0   0   0      0   0 PS

After a further 80 host seconds with A still physically
refused, DCS successfully executes another `PS`:

    PID PPID S WORDS TD RETRY PVM DETAIL ERR COMM
    5   2    2 2048  3 512   1      0   0 TRUE
    6   3    2 3072  0   0   0      0   0 PS

Thus an unrelated DCS shell remains schedulable, B stays safely
reusable, and A retains its PREVM owner and a capped retry
delay. There is no premature slot reclamation or busy spin.

The DCS operator then issues the *guest* privileged `HALT`
while A remains pinned. This uses DAIMOS's ordinary shutdown
interface, **not** a host interrupt or forced termination of
the simulator with writable media attached. Guest HALT reached
PC octal 022614 and SIMH exited normally.

## Physical memory witnesses after HALT

The versioned scratch SIMH command file reads the following
physical words only **after** the guest-initiated HALT, with
octal values from the actual `kcore.map`:

    025117: 401000016260  ; shared pending at capped 01000
    025121: 000000000000  ; B SIDL VM base released
    025122: 000000000005  ; B five real destroy attempts
    025123: 777777777777  ; B real mm_unpin completed
    025124: 000000760000  ; A exact VM base STILL PINNED
    025125: 000000000025  ; A 21 decimal real destroy attempts
    025126: 000000000000  ; A unpin never executed
    025127: 401000004636  ; before B: A capped 01000
    025130: 400020005300  ; after B: new initial 020
    025131: 000000005300  ; B first real refusal clock
    025132: 000000005320  ; B second real destroy clock
    025133: 000000000010  ; A eight decimal attempts at B pin

The actual B interval is octal `005320 - 005300 = 020`, or
16 qualified ticks. B resets the delay independently of A.

A's attempt counter advances from octal `010` at B's first
pin to octal `025` at operator HALT: **13 further real
VM-destroy attempts** while its ownership remains intact.
That modest count across the observed additional wait, along
with the capped pending word and responsive DCS shell, rules
out a retry busy-loop for this physical case. It does not
assert any real-time scheduling deadline beyond the measured
guest-clock retry interval.

Read-only `d6fsck -n 4` after operator HALT selected superblock
742 seq=3 state=CLEAN, 179196 blocks and 360 FCBs. This is
an orderly halt with a deliberately unreleased physical VM
owner; it is **not** a successful finalizer release of A.

## Testkit and remaining work

The physical driver and independent read-only verifier are:

    daimos-testkit/tests/validation/daimos-k04-permanent-pin-run-v1795.py
    daimos-testkit/tests/validation/daimos-k04-permanent-pin-verify-v1795.py

The verifier independently checks the actual CTY/DCS/SIMH
transcripts, the generated symbol map, both live owner-state
snapshots, exact provider extent accounting and retry clocks,
the retained shared pending flag, guest HALT and CLEAN D6FS.
Its positive self-test and seven corrupted negative controls
pass, and the real evidence passes.

K04 permanent real-MM refusal/operator recovery now PASSES for
this PREVM scenario. Other acceptance cases still OPEN include
physical FILE-close/SYNC failures and session HUP interaction,
dying-parent WAIT/adoption/PID reuse, and additional
multi-owner clock-wrap stress. The temporary-refusal SIDL/PREVM
orderings v1793/v1794 and single SIDL clock-wrap v1787 passed
separately. Do not close K04 in its entirety.
