# K04 v1780: physical SIDL rollback with real MM pin

Date: 2026-11-21. Status: **PASS** for one failed RUN with a
physically pinned VM extent, deferred SIDL discard and eventual
PID reuse. Other SIDL/PREVM orderings and K04 acceptance remain OPEN.

## Disposable test image

The image under `$TMPDIR/daimos-k04-v1780-sidl-once` derives from
the clean production v1773 PDP-6 256K boot baseline. Only generated
scratch `kcore-asm/proc_run.s` and `kcore-asm/proc_pdp6.s` were
instrumented; no production source or testkit changes were made.

In the scratch RUN hierarchy path, PID 5 is directed to the
existing `proc_run_hierarchy_bad` rollback path while the
test-only `k04_sidl_done_v1779` word is zero. After rollback
completes, the original hierarchy path is restored by a normal
conditional branch, so subsequent children can execute normally.

At the existing SIDL `vm_space_destroy` call site, the scratch
fixture pins the exact owned VM extent using real `mm_pin`.
The ordinary `vm_space_destroy` then encounters real MM refusal.
The fixture checks the exact VM base on subsequent retries, calls
real `mm_unpin` on its fifth attempt, and allows the unchanged
VM destroy/discard path to complete. Three scratch BSS words at
octal 025043..025045 hold base, attempts and completion. Native
DAS assembly, complete image build and initial D6FS CLEAN seq=1
passed. The extra code and words are **fixture-only**, not
production KCORE growth.

## Physical PDP-6 observations

After a fresh clean boot, CTY ROOT/DSH ran `TRUE`. The first
RUN was deliberately rejected after VM ownership was established.
The shell returned to its prompt after the deferred SIDL rollback
and real MM release. A read-only SIMH monitor examination showed:

    25043: 000000000000   ; pinned VM base released
    25044: 000000000005   ; five VM-destroy attempts
    25045: 777777777777   ; real MM unpin complete

After resuming, a subsequent `PS` reused PID 5 and showed all
teardown diagnostics zero:

    PID PPID S WORDS TD RETRY PVM DETAIL ERR COMM
    5   2    2 3072   0   0     0   0      0 PS

Guest `HALT` stopped at octal PC 022536. SIMH exited normally.
Final read-only `d6fsck -n 4` selected superblock 742 seq=3
CLEAN, 179196 blocks, 360 FCBs.

## Discarded trials and scope

The v1779 prototype injected the RUN failure on *every* PID 5
launch. It demonstrated real SIDL VM pin/release (five attempts,
base zero, done all ones), but made subsequent operator commands
fail and could not produce an orderly guest shutdown. Its dirty
disposable media was discarded; it is **not** acceptance evidence.

An initial v1780 boot accidentally sent `ROOT` twice, running a
spurious command before the intended TRUE; that run did not
activate the pin fixture. It halted cleanly and its media was
discarded. The accepted run used a freshly rebuilt clean disk,
one ROOT login and then TRUE, followed by the read-only fixture
inspection, PS reuse and guest HALT.

This passes one real **SIDL VM extent pin -> deferred rollback ->
unpin -> release -> PID reuse** sequence. It does not yet prove
permanent SIDL refusal, simultaneous SIDL/post-VM owner ordering,
PREVM EXIT refusal, clock wrap, FILE/HUP or dying-parent WAIT.
