# K04 v1781: physical PREVM EXIT VM pin and recovery

Date: 2026-11-21. Status: **PASS** for one real pinned-VM EXIT
PREVM refusal, deferred retry, eventual release and PID reuse.
The broader K04 acceptance matrix remains OPEN.

## Disposable fixture

Copy the clean v1773 256K PDP-6 boot baseline into a versioned
scratch build tree. Instrument only generated
`system/boot/pdp6/kcore-asm/proc_pdp6.s` at
`proc_finish_vm_retry_v997`, immediately before the ordinary
`vm_space_destroy` call. Production DAIMOS source and the external
testkit are not changed.

For slot 5 only, on its first EXIT PREVM attempt, read the owned
VM extent base from `1(12)` and call real `mm_pin`. Preserve that
base in a scratch BSS word. On each subsequent PREVM retry, verify
the descriptor still names the same base and increment the attempt
counter. On the fifth attempt call real `mm_unpin`, clear the saved
base and mark the fixture complete. Thereafter bypass the fixture
and call the unmodified `vm_space_destroy` path. Scratch symbols:

    k04_prevm_base_v1781   025037
    k04_prevm_calls_v1781  025040
    k04_prevm_done_v1781   025041

Native `das -F -C` PASS. Complete 256K image build PASS. Initial
read-only `d6fsck -n 4` CLEAN seq=1. The scratch instrumentation
is not a production KCORE size, ABI or fixed-memory change.

## Live PDP-6 acceptance

Boot the disposable image, log in as ROOT on CTY and run `TRUE`.
The shell returned to `#` after the EXIT PREVM reaper retried
physical MM refusal and the fifth attempt unpinned the VM extent.
Pause SIMH with Ctrl-E and examine the three scratch words:

    25037: 000000000000  ; pinned VM base cleared
    25040: 000000000005  ; five destroy attempts
    25041: 777777777777  ; unpin completed

Continue the guest and run `PS`. PID 5 was reused by PS with
TD=0 RETRY=0 PVM=0 DETAIL=0 ERR=0. The owner was therefore not
prematurely exposed as a live process during the refused release.
`HALT` stopped at octal PC 022532; SIMH exited normally.
Final read-only `d6fsck -n 4` selected superblock 742 seq=3
CLEAN (179196 blocks, 360 FCBs).

## Scope

This is a **real MM pin**, not a synthetic `MM_ERR_BUSY` return.
It demonstrates one PREVM EXIT owner retained across four refused
VM-destruction attempts, unpinned on the fifth, and subsequently
reaped with PID reuse. The fixture does not record intermediate
retry timestamps or prove every exponential delay value. It does
not test permanent refusal, two independent simultaneous owners,
17-bit clock wrap, FILE provider refusal, session HUP or
dying-parent WAIT. Those K04 cases remain OPEN.
